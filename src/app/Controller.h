#pragma once

#include "Settings.h"
#include "core/hardware/HardwareInfo.h"
#include "core/models/ModelManager.h"

#include <QObject>
#include <QPointer>
#include <QString>
#include <QThread>
#include <QTimer>

namespace dictapulse {

class AudioCapture;
class CleanupService;
class PlatformAdapter;
class SecretStore;
class TextProcessor;
class WhisperEngine;

/// Central state machine: wires global shortcuts → audio capture → whisper →
/// text injection. Exposes state and signals to QML.
class Controller : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(double level READ level NOTIFY levelChanged)
    Q_PROPERTY(QString lastTranscript READ lastTranscript NOTIFY lastTranscriptChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(QString detectedLanguage READ detectedLanguage NOTIFY detectedLanguageChanged)
    Q_PROPERTY(Settings* settings READ settings CONSTANT)
    Q_PROPERTY(ModelManager* models READ models CONSTANT)
    Q_PROPERTY(HardwareInfo* hardware READ hardware CONSTANT)

public:
    explicit Controller(Settings* settings,
                        ModelManager* models,
                        HardwareInfo* hardware,
                        PlatformAdapter* platform,
                        CleanupService* cleanup,
                        SecretStore* secrets,
                        QObject* parent = nullptr);
    ~Controller() override;

    QString state() const { return m_state; }
    QString statusText() const { return m_statusText; }
    double level() const { return m_level; }
    QString lastTranscript() const { return m_lastTranscript; }
    QString lastError() const { return m_lastError; }
    QString detectedLanguage() const { return m_detectedLanguage; }
    Settings* settings() const { return m_settings; }
    ModelManager* models() const { return m_models; }
    HardwareInfo* hardware() const { return m_hardware; }

    Q_INVOKABLE void toggleDictation();
    Q_INVOKABLE void startDictation();
    Q_INVOKABLE void stopDictation();
    Q_INVOKABLE void cancelDictation();
    Q_INVOKABLE void clearError();
    Q_INVOKABLE void applyShortcuts();
    Q_INVOKABLE bool ensureModelLoaded();
    Q_INVOKABLE QString keySequenceFromEvent(int key, int modifiers) const;
    Q_INVOKABLE QString modifierLabel(int modifiers) const;
    Q_INVOKABLE void showSettingsWindow();
    Q_INVOKABLE void quitApp();
    /// The window class/app-id of the currently focused window — used by the
    /// per-app rules page to show "Detected: <app>" and help authoring rules.
    Q_INVOKABLE QString activeWindowId() const;

signals:
    void stateChanged();
    void statusTextChanged();
    void levelChanged();
    void lastTranscriptChanged();
    void lastErrorChanged();
    void detectedLanguageChanged();
    void overlayRequested(bool show);
    void settingsRequested();
    void notify(const QString& title, const QString& body);

private slots:
    void onCaptureStarted();
    void onCaptureStopped();
    void onCaptureLevel(double rms);
    void onSilenceDetected();
    void onCaptureError(const QString& message);
    void onDictationShortcut();
    void onCancelShortcut();
    void onTraySettings();
    void onTrayQuit();

private:
    void setState(const QString& newState, const QString& status);
    void setError(const QString& err);
    void runTranscription();
    void finalizeInjection(const QString& text, const QString& mode, bool fallback);
    QString activeLanguage() const;
    void reloadDictionary();         // settings JSON → TextProcessor rules
    QString dictionaryPrompt() const; // dictionary terms → whisper initial_prompt
    QString resolveOutputMode() const; // per-app rule override or default

    Settings* m_settings;
    ModelManager* m_models;
    HardwareInfo* m_hardware;
    PlatformAdapter* m_platform;
    CleanupService* m_cleanup = nullptr;
    SecretStore* m_secrets = nullptr;
    AudioCapture* m_capture = nullptr;
    WhisperEngine* m_engine = nullptr;
    TextProcessor* m_text = nullptr;
    QThread m_workerThread;

    QString m_state = "idle";
    QString m_statusText = "Ready";
    double m_level = 0.0;
    QString m_lastTranscript;
    QString m_lastError;
    QString m_detectedLanguage;
    QString m_effectiveOutputMode; // resolved at dictation start (per-app aware)
    bool m_dictationActive = false;
};

} // namespace dictapulse
