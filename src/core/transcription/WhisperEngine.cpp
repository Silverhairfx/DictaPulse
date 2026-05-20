#include "WhisperEngine.h"

#include <QDebug>
#include <QFileInfo>

#include <whisper.h>

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
                                                bool translate)
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
    params.detect_language = autoDetect;
    params.temperature = 0.0f;
    params.no_speech_thold = 0.3f;     // less strict than default 0.6
    params.logprob_thold = -1.5f;      // accept lower-confidence tokens

    const QByteArray langBytes = language.toUtf8();
    if (autoDetect) {
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

    if (autoDetect) {
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
