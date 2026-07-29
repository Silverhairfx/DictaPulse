// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#include "Controller.h"

#include "Settings.h"
#include "core/audio/AudioCapture.h"
#include "core/cleanup/CleanupService.h"
#include "core/cleanup/SecretStore.h"
#include "core/hardware/HardwareInfo.h"
#include "core/models/ModelManager.h"
#include "core/profile/ProfileContext.h"
#include "core/profile/ProfileStats.h"
#include "core/text/TextProcessor.h"
#include "core/transcription/WhisperEngine.h"
#include "platform/PlatformAdapter.h"

#include <QCoreApplication>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeySequence>
#include <QRegularExpression>
#include <QSet>

#include <cstdio>

namespace dictapulse {

Controller::Controller(Settings* settings,
                       ModelManager* models,
                       HardwareInfo* hardware,
                       PlatformAdapter* platform,
                       CleanupService* cleanup,
                       SecretStore* secrets,
                       ProfileStats* stats,
                       QObject* parent)
    : QObject(parent)
    , m_settings(settings)
    , m_models(models)
    , m_hardware(hardware)
    , m_platform(platform)
    , m_cleanup(cleanup)
    , m_secrets(secrets)
    , m_stats(stats)
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

    // Profile: keep TextProcessor's rules (dictionary + voice templates) synced.
    connect(m_settings, &Settings::dictionaryChanged, this, &Controller::reloadProfileRules);
    connect(m_settings, &Settings::voiceTemplatesChanged, this, &Controller::reloadProfileRules);
    reloadProfileRules();

    // Auto-pick the first installed model if none selected.
    if (m_settings->activeModel().isEmpty()) {
        const QString id = m_models->defaultInstalledId();
        if (!id.isEmpty()) m_settings->setActiveModel(id);
    }

    // Auto-activate a model the moment it finishes downloading, if none is set.
    connect(m_models, &ModelManager::downloadFinished, this,
            [this](const QString& id, const QString&) {
                if (m_settings->activeModel().isEmpty()) {
                    m_settings->setActiveModel(id);
                }
                clearError();
            });

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
    if (m_lastError != err) {
        m_lastError = err;
        emit lastErrorChanged();
    }
    if (!err.isEmpty()) {
        setState("error", err);
        emit notify(tr("DictaPulse error"), err);
    }
}

void Controller::clearError()
{
    if (!m_lastError.isEmpty()) {
        m_lastError.clear();
        emit lastErrorChanged();
    }
}

QString Controller::activeLanguage() const
{
    if (m_settings->autoDetectLanguage()) return {};
    return m_settings->defaultLanguage();
}

QString Controller::activeWindowId() const
{
    return m_platform ? m_platform->activeWindowId() : QString();
}

void Controller::reloadProfileRules()
{
    // Dictionary + voice templates compose into one replacement list applied to
    // every transcript (see ProfileContext).
    m_text->setReplacements(ProfileContext::replacements(m_settings));
}

QString Controller::resolveOutputMode() const
{
    const QString fallback = m_settings->outputMode();
    if (!m_settings->perAppRulesEnabled()) return fallback;

    const QString app = m_platform ? m_platform->activeWindowId().toLower() : QString();
    if (app.isEmpty()) return fallback;

    // Rules are a JSON array of {match,mode}; first case-insensitive substring
    // match against the focused window's app-id/class wins.
    const QJsonDocument doc = QJsonDocument::fromJson(m_settings->perAppRules().toUtf8());
    if (!doc.isArray()) return fallback;
    for (const QJsonValue& v : doc.array()) {
        const QJsonObject o = v.toObject();
        const QString match = o.value("match").toString().trimmed().toLower();
        const QString mode = o.value("mode").toString();
        if (match.isEmpty() || mode.isEmpty()) continue;
        if (app.contains(match)) {
            std::fprintf(stderr, "[DictaPulse] per-app rule '%s' matched app '%s' → mode '%s'\n",
                         qUtf8Printable(match), qUtf8Printable(app), qUtf8Printable(mode));
            std::fflush(stderr);
            return mode;
        }
    }
    return fallback;
}

void Controller::applyShortcuts()
{
    if (!m_platform) return;
    m_platform->registerDictationShortcut(m_settings->shortcutDictate());
    m_platform->registerCancelShortcut(m_settings->shortcutCancel());
}

QString Controller::keySequenceFromEvent(int key, int modifiers) const
{
    if (key == 0) return {};
    return QKeySequence(key | modifiers).toString(QKeySequence::PortableText);
}

QString Controller::modifierLabel(int modifiers) const
{
    QStringList parts;
    if (modifiers & Qt::ControlModifier) parts << QStringLiteral("Ctrl");
    if (modifiers & Qt::AltModifier)     parts << QStringLiteral("Alt");
    if (modifiers & Qt::ShiftModifier)   parts << QStringLiteral("Shift");
    if (modifiers & Qt::MetaModifier)    parts << QStringLiteral("Meta");
    return parts.join(QLatin1Char('+'));
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
    clearError();
    if (!ensureModelLoaded()) return;

    m_dictationActive = true;
    // Capture focus context now, while the target window still holds focus -
    // per-app rules and usage stats both key off the app-id, which our overlay
    // must not perturb later.
    m_dictationApp = m_platform ? m_platform->activeWindowId() : QString();
    m_effectiveOutputMode = resolveOutputMode();
    if (m_settings->overlayEnabled()) emit overlayRequested(true);
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
    // Discard buffered samples - gain doesn't matter for a throwaway.
    (void)m_capture->takeFloatSamples(-1, false, 1.0);
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
    const double peakRms = m_capture->peakRms();
    auto samples = m_capture->takeFloatSamples(200,
                                               m_settings->autoGainEnabled(),
                                               m_settings->inputGain());
    const double durationSec = static_cast<double>(samples.size()) / 16000.0;
    const QString lang = activeLanguage();
    const bool autoDetect = m_settings->autoDetectLanguage();
    const int threads = m_settings->cpuThreads();
    const QString mode = m_effectiveOutputMode.isEmpty() ? m_settings->outputMode()
                                                         : m_effectiveOutputMode;
    const bool fallback = m_settings->clipboardFallback();
    // Profile → Whisper vocabulary bias (dictionary targets, dev jargon, cues).
    const QString dictPrompt = ProfileContext::whisperVocabulary(m_settings);
    TextProcessor::Options opts;
    opts.cleanup = m_settings->cleanupEnabled();
    opts.capitalize = m_settings->capitalizeSentences();
    opts.removeFillers = m_settings->removeFillerWords();
    opts.trailingSpace = m_settings->addTrailingSpace();
    opts.smartLists = m_settings->refineSmartLists();

    const bool translate = m_settings->translateToEnglish();
    // Restrict auto-detect to the languages the user actually enabled, so the
    // detector can't drift to a wrong-but-similar language (e.g. Arabic→Hebrew).
    const QStringList candidateLangs = autoDetect ? m_settings->enabledLanguages() : QStringList{};

    std::fprintf(stderr,
                 "[DictaPulse] captured %.2fs, peak RMS %.4f, threshold %.4f, translate=%d\n",
                 durationSec, peakRms, m_settings->vadThreshold(), translate);
    std::fflush(stderr);

    QMetaObject::invokeMethod(m_engine, [this, samples = std::move(samples), lang, autoDetect, threads, mode, fallback, opts, peakRms, durationSec, translate, candidateLangs, dictPrompt]() mutable {
        WhisperEngine::Result r = m_engine->transcribe(samples, lang, autoDetect, threads, translate, candidateLangs, dictPrompt);
        std::fprintf(stderr,
                     "[DictaPulse] whisper ok=%d lang='%s' text='%s'\n",
                     r.ok, qUtf8Printable(r.detectedLanguage), qUtf8Printable(r.text));
        std::fflush(stderr);
        QMetaObject::invokeMethod(this, [this, r, mode, fallback, opts, peakRms, durationSec, autoDetect]() {
            m_dictationActive = false;
            emit overlayRequested(false);
            if (r.ok && !r.detectedLanguage.isEmpty() && m_detectedLanguage != r.detectedLanguage) {
                m_detectedLanguage = r.detectedLanguage;
                emit detectedLanguageChanged();
            }
            if (!r.ok) {
                setError(r.error.isEmpty() ? tr("Transcription failed") : r.error);
                return;
            }

            const QString raw = r.text.trimmed();
            const QString diag = tr("%1s captured · peak %2")
                                     .arg(durationSec, 0, 'f', 1)
                                     .arg(peakRms, 0, 'f', 3);

            if (raw.isEmpty()) {
                // Surface the empty attempt in the transcript box so the user
                // can see something *did* happen, not just stare at a placeholder.
                m_lastTranscript = tr("[empty transcript] %1\nlanguage attempted: %2\n"
                                      "Whisper returned no text. Tips: speak for at least "
                                      "2 seconds, switch to the Base or Small model, or "
                                      "lower the voice threshold in Advanced.")
                                       .arg(diag,
                                            r.detectedLanguage.isEmpty()
                                                ? (autoDetect ? tr("auto-detect") : m_settings->defaultLanguage())
                                                : r.detectedLanguage);
                emit lastTranscriptChanged();
                if (peakRms < 0.003) {
                    setState("idle", tr("No audio (%1) - check mic input").arg(diag));
                    emit notify(tr("DictaPulse"),
                                tr("Microphone seems silent. Verify your default input in KDE System Settings → Audio → Recording."));
                } else {
                    setState("idle", tr("Whisper returned nothing (%1) - try a longer phrase or a bigger model").arg(diag));
                    emit notify(tr("DictaPulse"),
                                tr("Captured audio (peak %1) but Whisper produced no text. Speak for ≥2s, or try the Base/Small model.")
                                    .arg(peakRms, 0, 'f', 3));
                }
                return;
            }

            // Route the raw transcript through the configured cleanup provider.
            //   none  → inject as-is        rules → TextProcessor (offline)
            //   local → OpenAI-compat LLM   remote → Anthropic/OpenAI/custom
            const QString provider = m_settings->cleanupProvider();
            const QString cleanupLang = r.detectedLanguage.isEmpty()
                                            ? m_settings->defaultLanguage()
                                            : r.detectedLanguage;

            if (m_cleanup && (provider == QLatin1String("local") || provider == QLatin1String("remote"))) {
                setState("cleaning", tr("Cleaning…"));
                CleanupService::Config cfg;
                cfg.provider = provider;
                cfg.localEndpoint = m_settings->cleanupLocalEndpoint();
                cfg.localModel = m_settings->cleanupLocalModel();
                cfg.remoteProvider = m_settings->cleanupRemoteProvider();
                cfg.remoteModel = m_settings->cleanupRemoteModel();
                cfg.remoteEndpoint = m_settings->cleanupRemoteEndpoint();
                // Append the user's Profile so the LLM honors tone, refine
                // rules, dictionary, templates, and dev context too.
                cfg.systemPrompt = m_settings->cleanupSystemPrompt()
                                   + ProfileContext::systemPromptAugmentation(m_settings);
                if (provider == QLatin1String("remote") && m_secrets)
                    cfg.apiKey = m_secrets->key(m_settings->cleanupRemoteProvider());
                // cfg.vocabulary is filled once the vocabulary feature lands.

                // One request in flight at a time; clear any prior handlers.
                disconnect(m_cleanup, nullptr, this, nullptr);
                connect(m_cleanup, &CleanupService::cleaned, this,
                        [this, mode, fallback, opts](const QString& text) {
                            disconnect(m_cleanup, nullptr, this, nullptr);
                            QString out = text;
                            if (opts.trailingSpace && !out.endsWith(QLatin1Char(' ')))
                                out.append(QLatin1Char(' '));
                            finalizeInjection(out, mode, fallback);
                        });
                connect(m_cleanup, &CleanupService::failed, this,
                        [this, raw, cleanupLang, mode, fallback, opts](const QString& err) {
                            disconnect(m_cleanup, nullptr, this, nullptr);
                            std::fprintf(stderr,
                                         "[DictaPulse] cleanup failed: %s - falling back to rules\n",
                                         qUtf8Printable(err));
                            std::fflush(stderr);
                            emit notify(tr("DictaPulse"),
                                        tr("Cleanup failed (%1) - inserted lightly-cleaned text").arg(err));
                            finalizeInjection(m_text->process(raw, cleanupLang, opts), mode, fallback);
                        });
                m_cleanup->process(raw, cleanupLang, cfg);
                return;
            }

            QString out;
            if (provider == QLatin1String("none")) {
                out = raw;
                if (opts.trailingSpace && !out.endsWith(QLatin1Char(' ')))
                    out.append(QLatin1Char(' '));
            } else { // "rules" (default) or anything unrecognized
                out = m_text->process(raw, cleanupLang, opts);
            }
            finalizeInjection(out, mode, fallback);
        }, Qt::QueuedConnection);
    }, Qt::QueuedConnection);
}

void Controller::finalizeInjection(const QString& text, const QString& mode, bool fallback)
{
    m_lastTranscript = text;
    emit lastTranscriptChanged();

    // Usage stats: count words in what we actually produced, attributed to the
    // app that was focused when dictation began. Feeds the Profile dashboard.
    if (m_stats) {
        const int words = text.split(QRegularExpression(R"(\s+)"), Qt::SkipEmptyParts).size();
        m_stats->record(words, m_dictationApp);
    }

    const auto inj = m_platform->injectText(text, mode, fallback);
    const char* injStr = inj == PlatformAdapter::InjectResult::Inserted ? "Inserted"
                       : inj == PlatformAdapter::InjectResult::ClipboardOnly ? "ClipboardOnly"
                       : "Failed";
    std::fprintf(stderr,
                 "[DictaPulse] inject mode='%s' fallback=%d result=%s chars=%lld\n",
                 qUtf8Printable(mode), fallback, injStr,
                 static_cast<long long>(text.size()));
    std::fflush(stderr);
    QString detail;
    switch (inj) {
    case PlatformAdapter::InjectResult::Inserted:
        detail = tr("Inserted (%1 chars)").arg(text.size());
        break;
    case PlatformAdapter::InjectResult::ClipboardOnly:
        detail = tr("Copied to clipboard");
        emit notify(tr("DictaPulse"), tr("Copied %1 characters to clipboard").arg(text.size()));
        break;
    case PlatformAdapter::InjectResult::Failed:
        detail = tr("Insertion failed");
        emit notify(tr("DictaPulse"), tr("Could not insert text - copied to clipboard"));
        break;
    }
    setState("idle", detail);
}

} // namespace dictapulse
