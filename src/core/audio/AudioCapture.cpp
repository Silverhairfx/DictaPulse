// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#include "AudioCapture.h"

#include "InputDeviceSelector.h"

#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioSource>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QMediaDevices>

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace dictapulse {

namespace {
double computeRms(const qint16* samples, qsizetype count)
{
    if (count <= 0) return 0.0;
    double sumSq = 0.0;
    for (qsizetype i = 0; i < count; ++i) {
        const double s = static_cast<double>(samples[i]) / 32768.0;
        sumSq += s * s;
    }
    return std::sqrt(sumSq / static_cast<double>(count));
}
} // namespace

AudioCapture::AudioCapture(QObject* parent)
    : QObject(parent)
{
    m_silenceTimer.setInterval(100);
    connect(&m_silenceTimer, &QTimer::timeout, this, &AudioCapture::onSilenceTick);
    m_maxTimer.setSingleShot(true);
    connect(&m_maxTimer, &QTimer::timeout, this, &AudioCapture::stop);
}

AudioCapture::~AudioCapture()
{
    stop();
}

// Test hook: DICTAPULSE_TEST_AUDIO=<file.wav> replaces the microphone with a
// recorded file (16-bit PCM, any rate or channel count), so the full
// transcription and insertion path can be exercised without a person speaking.
bool AudioCapture::startFromFile(const QString& path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        emit error(tr("Cannot open test audio %1").arg(path));
        return false;
    }
    const QByteArray raw = f.readAll();
    auto u16 = [&raw](int off) { return static_cast<quint16>(static_cast<quint8>(raw[off]) | (static_cast<quint8>(raw[off + 1]) << 8)); };
    auto u32 = [&u16](int off) { return static_cast<quint32>(u16(off)) | (static_cast<quint32>(u16(off + 2)) << 16); };
    if (raw.size() < 44 || raw.mid(0, 4) != "RIFF" || raw.mid(8, 4) != "WAVE") {
        emit error(tr("Test audio is not a WAV file"));
        return false;
    }
    int channels = 1, rate = kSampleRate, bits = 16, dataOff = -1, dataLen = 0;
    for (int off = 12; off + 8 <= raw.size();) {
        const QByteArray id = raw.mid(off, 4);
        const int len = static_cast<int>(u32(off + 4));
        if (id == "fmt ") {
            channels = u16(off + 10);
            rate = static_cast<int>(u32(off + 12));
            bits = u16(off + 22);
        } else if (id == "data") {
            dataOff = off + 8;
            dataLen = std::min<int>(len, static_cast<int>(raw.size()) - dataOff);
            break;
        }
        off += 8 + len + (len & 1);
    }
    if (dataOff < 0 || bits != 16 || channels < 1 || rate <= 0) {
        emit error(tr("Test audio must be 16-bit PCM WAV"));
        return false;
    }
    const auto* in = reinterpret_cast<const qint16*>(raw.constData() + dataOff);
    const qsizetype frames = dataLen / (2 * channels);
    QVector<qint16> mono(frames);
    for (qsizetype i = 0; i < frames; ++i) {
        int sum = 0;
        for (int c = 0; c < channels; ++c) sum += in[i * channels + c];
        mono[i] = static_cast<qint16>(sum / channels);
    }
    const qsizetype outFrames = static_cast<qsizetype>(static_cast<double>(frames) * kSampleRate / rate);
    m_pcm.clear();
    m_pcm.resize(outFrames * 2);
    auto* out = reinterpret_cast<qint16*>(m_pcm.data());
    for (qsizetype i = 0; i < outFrames; ++i) {
        const double pos = static_cast<double>(i) * rate / kSampleRate;
        const qsizetype i0 = std::min<qsizetype>(static_cast<qsizetype>(pos), frames - 1);
        const qsizetype i1 = std::min<qsizetype>(i0 + 1, frames - 1);
        const double frac = pos - static_cast<double>(i0);
        out[i] = static_cast<qint16>(mono[i0] * (1.0 - frac) + mono[i1] * frac);
    }
    m_peakRms = computeRms(out, outFrames);
    m_vadThreshold = 0.005;
    m_recording = true;
    std::fprintf(stderr, "[DictaPulse] test audio: %s (%.1fs)\n", qUtf8Printable(path),
                 static_cast<double>(outFrames) / kSampleRate);
    std::fflush(stderr);
    emit started();
    return true;
}

void AudioCapture::start(double vadThreshold, int silenceMs, int maxSeconds)
{
    if (m_recording) return;

    const QString testAudio = qEnvironmentVariable("DICTAPULSE_TEST_AUDIO");
    if (!testAudio.isEmpty()) {
        startFromFile(testAudio);
        return;
    }

    QAudioFormat fmt;
    fmt.setSampleRate(kSampleRate);
    fmt.setChannelCount(kChannels);
    fmt.setSampleFormat(QAudioFormat::Int16);

    QAudioDevice device = QMediaDevices::defaultAudioInput();
    {
        const QList<QAudioDevice> inputs = QMediaDevices::audioInputs();
        QVector<InputCandidate> cands;
        cands.reserve(inputs.size());
        for (const QAudioDevice& d : inputs) {
            cands.push_back({ d.id(), d.description(), d.isDefault() });
        }
        const int idx = pickInputDevice(cands, m_explicitDevice, m_preferredPatterns);
        if (idx >= 0) device = inputs.at(idx);
    }
    m_lastDevice = device.description();
    std::fprintf(stderr, "[DictaPulse] input device: %s\n", qUtf8Printable(m_lastDevice));
    std::fflush(stderr);
    if (device.isNull()) {
        emit error(tr("No audio input device available."));
        return;
    }
    if (!device.isFormatSupported(fmt)) {
        emit error(tr("Default microphone does not support 16 kHz mono PCM."));
        return;
    }

    m_pcm.clear();
    m_pcm.reserve(kSampleRate * 2 * 30); // 30s prealloc
    m_vadThreshold = vadThreshold;
    m_silenceMs = silenceMs;
    m_peakRms = 0.0;
    m_lastVoiceAtMs = QDateTime::currentMSecsSinceEpoch();

    delete m_source;
    m_source = new QAudioSource(device, fmt, this);
    m_source->setBufferSize(kSampleRate * 2 / 10); // ~100ms

    m_io = m_source->start();
    if (!m_io) {
        emit error(tr("Failed to start audio source."));
        delete m_source;
        m_source = nullptr;
        return;
    }
    connect(m_io.data(), &QIODevice::readyRead, this, &AudioCapture::onReadyRead);

    m_recording = true;
    m_silenceTimer.start();
    if (maxSeconds > 0) m_maxTimer.start(maxSeconds * 1000);
    emit started();
}

void AudioCapture::stop()
{
    if (!m_recording) return;
    m_silenceTimer.stop();
    m_maxTimer.stop();
    if (m_source) {
        m_source->stop();
        m_source->deleteLater();
        m_source = nullptr;
    }
    m_io.clear();
    m_recording = false;
    emit stopped();
}

void AudioCapture::onReadyRead()
{
    if (!m_io) return;
    const QByteArray chunk = m_io->readAll();
    if (chunk.isEmpty()) return;
    m_pcm.append(chunk);

    const auto* samples = reinterpret_cast<const qint16*>(chunk.constData());
    const qsizetype count = chunk.size() / static_cast<qsizetype>(sizeof(qint16));
    const double rms = computeRms(samples, count);
    if (rms > m_peakRms) m_peakRms = rms;
    emit levelChanged(rms);
    if (rms > m_vadThreshold) {
        m_lastVoiceAtMs = QDateTime::currentMSecsSinceEpoch();
    }
}

double AudioCapture::durationSeconds() const
{
    const qsizetype bytes = m_pcm.size();
    return static_cast<double>(bytes / 2) / static_cast<double>(kSampleRate);
}

void AudioCapture::onSilenceTick()
{
    if (!m_recording) return;
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now - m_lastVoiceAtMs > m_silenceMs && m_pcm.size() > kSampleRate * 2 / 2) {
        emit silenceDetected();
    }
}

std::vector<float> AudioCapture::takeFloatSamples(int keepTrailingSilenceMs,
                                                  bool autoGain,
                                                  double manualGain)
{
    const auto* samples = reinterpret_cast<const qint16*>(m_pcm.constData());
    const qsizetype total = m_pcm.size() / static_cast<qsizetype>(sizeof(qint16));
    std::vector<float> out;
    out.resize(static_cast<size_t>(total));
    for (qsizetype i = 0; i < total; ++i) {
        out[static_cast<size_t>(i)] = static_cast<float>(samples[i]) / 32768.0f;
    }
    m_pcm.clear();

    if (out.empty()) return out;

    if (keepTrailingSilenceMs >= 0) {
        const int windowSamples = kSampleRate * 20 / 1000;
        const double trimThreshold = std::max(0.001, m_vadThreshold * 0.6);
        size_t lastVoiceEnd = out.size();
        for (size_t end = out.size(); end >= static_cast<size_t>(windowSamples); end -= windowSamples) {
            const size_t start = end - windowSamples;
            double sumSq = 0.0;
            for (size_t i = start; i < end; ++i) sumSq += out[i] * out[i];
            const double rms = std::sqrt(sumSq / windowSamples);
            if (rms > trimThreshold) {
                lastVoiceEnd = end;
                break;
            }
        }
        const size_t keepSamples = static_cast<size_t>(kSampleRate * keepTrailingSilenceMs / 1000);
        const size_t newSize = std::min(out.size(), lastVoiceEnd + keepSamples);
        if (newSize < out.size()) out.resize(newSize);
    }

    // Auto-gain: normalize peak to whisper's training range. Quiet KDE/PipeWire
    // mics (peak <0.05) make whisper return empty even on clear speech.
    float peakIn = 0.0f;
    for (float s : out) peakIn = std::max(peakIn, std::fabs(s));

    constexpr float kTargetPeak = 0.5f;   // -6 dBFS
    constexpr float kMaxAutoGain = 20.0f; // +26 dB ceiling
    constexpr float kMinPeakToBoost = 0.001f; // don't amplify pure silence

    float autoG = 1.0f;
    if (autoGain && peakIn >= kMinPeakToBoost) {
        autoG = std::min(kMaxAutoGain, kTargetPeak / peakIn);
        if (autoG < 1.0f) autoG = 1.0f;  // never attenuate, only boost
    }
    const float manualG = static_cast<float>(std::max(0.0, manualGain));
    const float totalG = autoG * manualG;
    if (std::fabs(totalG - 1.0f) > 1e-3f) {
        for (auto& s : out) {
            float v = s * totalG;
            if (v > 1.0f) v = 1.0f;
            else if (v < -1.0f) v = -1.0f;
            s = v;
        }
    }
    float peakOut = 0.0f;
    for (float s : out) peakOut = std::max(peakOut, std::fabs(s));
    std::fprintf(stderr,
                 "[DictaPulse] gain: peakIn=%.3f autoG=%.2fx manualG=%.2fx peakOut=%.3f samples=%zu\n",
                 peakIn, autoG, manualG, peakOut, out.size());
    std::fflush(stderr);
    return out;
}

} // namespace dictapulse
