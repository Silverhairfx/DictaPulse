// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#include "WhisperEngine.h"

#include <QDebug>
#include <QFileInfo>

#include <whisper.h>

#include <cstdio>
#include <vector>

namespace dictapulse {

WhisperEngine::WhisperEngine(QObject* parent)
    : QObject(parent)
{
}

WhisperEngine::~WhisperEngine()
{
    unloadModel();
}

void WhisperEngine::unloadModel()
{
    if (m_ctx) {
        whisper_free(m_ctx);
        m_ctx = nullptr;
    }
    m_path.clear();
}

bool WhisperEngine::loadModel(const QString& path, bool useGpu)
{
    if (m_ctx && m_path == path && m_gpuRequested == useGpu) return true;
    unloadModel();

    if (!QFileInfo::exists(path)) {
        emit modelLoadFailed(path, tr("Model file not found"));
        return false;
    }

    whisper_context_params cparams = whisper_context_default_params();
    cparams.use_gpu = useGpu;

    m_ctx = whisper_init_from_file_with_params(path.toUtf8().constData(), cparams);
    if (!m_ctx) {
        emit modelLoadFailed(path, tr("whisper_init_from_file failed"));
        return false;
    }

    m_path = path;
    m_gpuRequested = useGpu;
    emit modelLoaded(path);
    return true;
}

WhisperEngine::Result WhisperEngine::transcribe(const std::vector<float>& samples,
                                                const QString& language,
                                                bool autoDetect,
                                                int threads,
                                                bool translate,
                                                const QStringList& candidateLangs,
                                                const QString& initialPrompt)
{
    Result result;
    if (!m_ctx) {
        result.error = tr("No model loaded");
        return result;
    }
    if (samples.empty()) {
        result.error = tr("Empty audio buffer");
        return result;
    }

    // Constrained auto-detect: whisper's open-set detector confuses Arabic with
    // Hebrew/Farsi/Urdu on short colloquial clips. When the user has a small set
    // of enabled languages, restrict detection to that set — compute the mel,
    // ask whisper for per-language probabilities, and force the highest-scoring
    // *enabled* language. This keeps en/ar working while never picking 'he'.
    QString forcedLang;
    if (autoDetect && candidateLangs.size() == 1) {
        // Only one enabled language: there's nothing to detect between — force it.
        // (Falling through to open-set "auto" here is what made single-language
        // setups silently misbehave.)
        forcedLang = candidateLangs.first();
        std::fprintf(stderr, "[DictaPulse] single enabled language: forcing '%s'\n",
                     qUtf8Printable(forcedLang));
        std::fflush(stderr);
    } else if (autoDetect && candidateLangs.size() >= 2) {
        const int nt = threads > 0 ? threads : 4;
        if (whisper_pcm_to_mel(m_ctx, samples.data(), static_cast<int>(samples.size()), nt) == 0) {
            std::vector<float> probs(static_cast<size_t>(whisper_lang_max_id()) + 1, 0.0f);
            whisper_lang_auto_detect(m_ctx, 0, nt, probs.data());
            float bestProb = -1.0f;
            for (const QString& code : candidateLangs) {
                const int id = whisper_lang_id(code.toUtf8().constData());
                if (id >= 0 && id < static_cast<int>(probs.size()) && probs[id] > bestProb) {
                    bestProb = probs[id];
                    forcedLang = code;
                }
            }
            std::fprintf(stderr, "[DictaPulse] constrained-detect: picked '%s' (p=%.3f) from %s\n",
                         qUtf8Printable(forcedLang), bestProb,
                         qUtf8Printable(candidateLangs.join(",")));
            std::fflush(stderr);
        }
    }

    whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    params.n_threads = threads > 0 ? threads : 4;
    params.translate = translate;
    params.print_progress = false;
    params.print_realtime = false;
    params.print_timestamps = false;
    params.print_special = false;
    params.no_context = true;
    params.single_segment = false;
    params.suppress_blank = true;
    // suppress_nst is aggressive on short utterances and rushed speech — many
    // valid transcriptions get filtered out as 'non-speech tokens'. Off.
    params.suppress_nst = false;
    // CRITICAL: detect_language=true means "detect language and STOP" (whisper.cpp
    // returns 0 with no segments). For detect+transcribe we leave this false and
    // rely on language="auto" below to trigger detection that continues into
    // transcription. Setting both produces empty transcripts on every auto-detect.
    params.detect_language = false;
    params.temperature = 0.0f;
    params.no_speech_thold = 0.3f;     // less strict than default 0.6
    params.logprob_thold = -1.5f;      // accept lower-confidence tokens

    // Vocabulary / context biasing. whisper.cpp conditions decoding on this text
    // as if it preceded the audio, nudging spelling toward these words (names,
    // jargon, the user's personal dictionary). Kept alive for the whisper_full call.
    const QByteArray promptBytes = initialPrompt.trimmed().toUtf8();
    if (!promptBytes.isEmpty()) {
        params.initial_prompt = promptBytes.constData();
    }

    const QByteArray langBytes = language.toUtf8();
    const QByteArray forcedBytes = forcedLang.toUtf8();
    if (!forcedLang.isEmpty()) {
        // Constrained detection resolved the language to an enabled one.
        params.language = forcedBytes.constData();
    } else if (autoDetect) {
        // 'auto' is whisper.cpp's documented sentinel for "no prior". Without
        // this whisper.cpp keeps the default ('en'), which silently biases the
        // language detector toward English on short clips.
        params.language = "auto";
    } else if (!language.isEmpty()) {
        params.language = langBytes.constData();
    }

    const int rc = whisper_full(m_ctx, params, samples.data(), static_cast<int>(samples.size()));
    if (rc != 0) {
        result.error = tr("whisper_full failed (%1)").arg(rc);
        return result;
    }

    const int n = whisper_full_n_segments(m_ctx);
    QString text;
    text.reserve(256);
    for (int i = 0; i < n; ++i) {
        text += QString::fromUtf8(whisper_full_get_segment_text(m_ctx, i));
    }

    if (!forcedLang.isEmpty()) {
        result.detectedLanguage = forcedLang;
    } else if (autoDetect) {
        const int langId = whisper_full_lang_id(m_ctx);
        if (langId >= 0) {
            result.detectedLanguage = QString::fromUtf8(whisper_lang_str(langId));
        }
    } else {
        result.detectedLanguage = language;
    }
    result.text = text.trimmed();
    result.ok = true;
    return result;
}

} // namespace dictapulse
