#pragma once

#include <QByteArray>
#include <QIODevice>
#include <QObject>
#include <QPointer>
#include <QTimer>

#include <vector>

class QAudioSource;

namespace dictapulse {

/// Captures 16 kHz mono PCM s16 audio from the default input.
/// Emits running RMS levels for the overlay and silence detection.
class AudioCapture : public QObject {
    Q_OBJECT

public:
    static constexpr int kSampleRate = 16000;
    static constexpr int kChannels = 1;

    explicit AudioCapture(QObject* parent = nullptr);
    ~AudioCapture() override;

    bool isRecording() const { return m_recording; }

    /// Returns interleaved float32 samples in [-1, 1] for whisper.
    std::vector<float> takeFloatSamples();

    double peakRms() const { return m_peakRms; }
    double durationSeconds() const;

signals:
    void levelChanged(double rms);
    void silenceDetected();
    void error(const QString& message);
    void started();
    void stopped();

public slots:
    void start(double vadThreshold, int silenceMs, int maxSeconds);
    void stop();

private slots:
    void onReadyRead();
    void onSilenceTick();

private:
    QAudioSource* m_source = nullptr;
    QPointer<QIODevice> m_io;
    QByteArray m_pcm;
    QTimer m_silenceTimer;
    QTimer m_maxTimer;
    double m_vadThreshold = 0.012;
    int m_silenceMs = 1200;
    qint64 m_lastVoiceAtMs = 0;
    double m_peakRms = 0.0;
    bool m_recording = false;
};

} // namespace dictapulse
