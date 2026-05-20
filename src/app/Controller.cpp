#include "Controller.h"

#include "Settings.h"
#include "core/audio/AudioCapture.h"
#include "core/hardware/HardwareInfo.h"
#include "core/models/ModelManager.h"
#include "core/text/TextProcessor.h"
#include "core/transcription/WhisperEngine.h"
#include "platform/PlatformAdapter.h"

#include <QCoreApplication>
#include <QDebug>

namespace dictapulse {

Controller::Controller(Settings* settings,
                       ModelManager* models,
                       HardwareInfo* hardware,
                       PlatformAdapter* platform,
                       QObject* parent)
    : QObject(parent)
    , m_settings(settings)
    , m_models(models)
    , m_hardware(hardware)
    , m_platform(platform)
    , m_capture(new AudioCapture(this))
    , m_engine(new WhisperEngine())
    , m_text(new TextProcessor(this))
{
    m_engine->moveToThread(&m_workerThread);
    m_workerThread.start();

    connect(m_capture, &AudioCapture::started, this, &Controller::onCaptureStarted);
    connect(m_capture, &AudioCapture::stopped, this, &Controller::onCaptureStopped);
    connect(m_capture, &AudioCapture::levelChanged, this, &Controller::onCaptureLevel);
    connect(m_capture, &AudioCapture::silenceDetected, this, &Controller::onSilenceDetected);
    connect(m_capture, &AudioCapture::error, this, &Controller::onCaptureError);

    connect(m_platform, &PlatformAdapter::dictationShortcutPressed,
            this, &Controller::onDictationShortcut);
    connect(m_platform, &PlatformAdapter::cancelShortcutPressed,
            this, &Controller::onCancelShortcut);
    connect(m_platform, &PlatformAdapter::traySettingsRequested,
            this, &Controller::onTraySettings);
    connect(m_platform, &PlatformAdapter::trayQuitRequested,
            this, &Controller::onTrayQuit);

    // Auto-pick the first installed model if none selected.
    if (m_settings->activeModel().isEmpty()) {
        const QString id = m_models->defaultInstalledId();
        if (!id.isEmpty()) m_settings->setActiveModel(id);
    }

    applyShortcuts();
}

Controller::~Controller()
{
    m_workerThread.quit();
    m_workerThread.wait();
    delete m_engine;
}

void Controller::setState(const QString& newState, const QString& status)
{
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged();
    }
    if (m_statusText != status) {
        m_statusText = status;
        emit statusTextChanged();
    }
    if (m_platform) m_platform->setTrayState(newState, status);
}

void Controller::setError(const QString& err)
{
    m_lastError = err;
    emit lastErrorChanged();
    if (!err.isEmpty()) {
        setState("error", err);
        emit notify(tr("DictaPulse error"), err);
    }
}

QString Controller::activeLanguage() const
{
    if (m_settings->autoDetectLanguage()) return {};
    return m_settings->defaultLanguage();
}

void Controller::applyShortcuts()
{
    if (!m_platform) return;
    m_platform->registerDictationShortcut(m_settings->shortcutDictate());
    m_platform->registerCancelShortcut(m_settings->shortcutCancel());
}

bool Controller::ensureModelLoaded()
{
    const QString id = m_settings->activeModel();
    if (id.isEmpty()) {
        setError(tr("No model selected. Open the Models page to download one."));
        return false;
    }
    const QString path = m_models->localPathFor(id);
    if (path.isEmpty() || !m_models->isInstalled(id)) {
        setError(tr("Model '%1' not installed.").arg(id));
        return false;
    }
    if (m_engine->isLoaded() && m_engine->loadedModelPath() == path) return true;

    setState("loading-model", tr("Loading model…"));
    const bool useGpu = m_settings->backendMode() != "cpu";
    bool ok = false;
    QMetaObject::invokeMethod(
        m_engine,
        [this, path, useGpu]() { return m_engine->loadModel(path, useGpu); },
        Qt::BlockingQueuedConnection,
        &ok);
    if (!ok) {
        setError(tr("Failed to load model: %1").arg(path));
        return false;
    }
    setState("idle", tr("Ready"));
    return true;
}

void Controller::onDictationShortcut()
{
    toggleDictation();
}

void Controller::onCancelShortcut()
{
    cancelDictation();
}

void Controller::onTraySettings()
{
    emit settingsRequested();
}

void Controller::onTrayQuit()
{
    quitApp();
}

void Controller::showSettingsWindow()
{
    emit settingsRequested();
}

void Controller::quitApp()
{
    QCoreApplication::quit();
}

void Controller::toggleDictation()
{
    if (m_dictationActive) {
        stopDictation();
    } else {
        startDictation();
    }
}

void Controller::startDictation()
{
    if (m_dictationActive) return;
    if (!ensureModelLoaded()) return;

    m_dictationActive = true;
    emit overlayRequested(true);
    m_capture->start(m_settings->vadThreshold(),
                     m_settings->silenceMs(),
                     m_settings->maxRecordingSeconds());
}

void Controller::stopDictation()
{
    if (!m_dictationActive) return;
    m_capture->stop();
}

void Controller::cancelDictation()
{
    if (!m_dictationActive) {
        emit overlayRequested(false);
        return;
    }
    m_dictationActive = false;
    m_capture->stop();
    // Discard buffered samples.
    (void)m_capture->takeFloatSamples();
    setState("idle", tr("Cancelled"));
    emit overlayRequested(false);
}

void Controller::onCaptureStarted()
{
    setState("listening", tr("Listening…"));
}

void Controller::onCaptureLevel(double rms)
{
    m_level = rms;
    emit levelChanged();
}

void Controller::onCaptureError(const QString& message)
{
    m_dictationActive = false;
    emit overlayRequested(false);
    setError(message);
}

void Controller::onSilenceDetected()
{
    if (!m_dictationActive) return;
    if (m_settings->dictationMode() == "ptt") return; // push-to-talk ignores auto-stop
    stopDictation();
}

void Controller::onCaptureStopped()
{
    if (!m_dictationActive) return;
    setState("processing", tr("Transcribing…"));
    runTranscription();
}

void Controller::runTranscription()
{
    auto samples = m_capture->takeFloatSamples();
    const QString lang = activeLanguage();
    const bool autoDetect = m_settings->autoDetectLanguage();
    const int threads = m_settings->cpuThreads();
    const QString mode = m_settings->outputMode();
    const bool fallback = m_settings->clipboardFallback();
    TextProcessor::Options opts;
    opts.cleanup = m_settings->cleanupEnabled();
    opts.capitalize = m_settings->capitalizeSentences();
    opts.removeFillers = m_settings->removeFillerWords();
    opts.trailingSpace = m_settings->addTrailingSpace();

    QMetaObject::invokeMethod(m_engine, [this, samples = std::move(samples), lang, autoDetect, threads, mode, fallback, opts]() mutable {
        WhisperEngine::Result r = m_engine->transcribe(samples, lang, autoDetect, threads);
        QMetaObject::invokeMethod(this, [this, r, mode, fallback, opts]() {
            m_dictationActive = false;
            emit overlayRequested(false);
            if (!r.ok) {
                setError(r.error.isEmpty() ? tr("Transcription failed") : r.error);
                return;
            }
            const QString polished = m_text->process(r.text, r.detectedLanguage, opts);
            m_lastTranscript = polished;
            emit lastTranscriptChanged();

            const auto inj = m_platform->injectText(polished, mode, fallback);
            QString detail;
            switch (inj) {
            case PlatformAdapter::InjectResult::Inserted:
                detail = tr("Inserted (%1 chars)").arg(polished.size());
                break;
            case PlatformAdapter::InjectResult::ClipboardOnly:
                detail = tr("Copied to clipboard");
                emit notify(tr("DictaPulse"), tr("Copied %1 characters to clipboard").arg(polished.size()));
                break;
            case PlatformAdapter::InjectResult::Failed:
                detail = tr("Insertion failed");
                emit notify(tr("DictaPulse"), tr("Could not insert text — copied to clipboard"));
                break;
            }
            setState("idle", detail);
        }, Qt::QueuedConnection);
    }, Qt::QueuedConnection);
}

} // namespace dictapulse
