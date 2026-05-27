#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

#include <memory>
#include <vector>

struct whisper_context;

namespace dictapulse {

class WhisperEngine : public QObject {
    Q_OBJECT

public:
    struct Result {
        QString text;
        QString detectedLanguage;
        bool ok = false;
        QString error;
    };

    explicit WhisperEngine(QObject* parent = nullptr);
    ~WhisperEngine() override;

    bool loadModel(const QString& path, bool useGpu);
    void unloadModel();
    bool isLoaded() const { return m_ctx != nullptr; }
    QString loadedModelPath() const { return m_path; }

    /// Runs synchronously. Call from a worker thread.
    /// `translate`: optional Whisper feature that transcribes-and-translates
    /// from the spoken language into English. Off by default — DictaPulse is
    /// a transcription tool, not a translator.
    Result transcribe(const std::vector<float>& samples,
                      const QString& language,
                      bool autoDetect,
                      int threads,
                      bool translate = false,
                      const QStringList& candidateLangs = {},
                      const QString& initialPrompt = {});

signals:
    void modelLoaded(const QString& path);
    void modelLoadFailed(const QString& path, const QString& error);

private:
    whisper_context* m_ctx = nullptr;
    QString m_path;
    bool m_gpuRequested = false;
};

} // namespace dictapulse
