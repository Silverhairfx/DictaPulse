// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#pragma once

#include <QObject>
#include <QSettings>
#include <QStringList>

namespace dictapulse {

class Settings : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString shortcutDictate READ shortcutDictate WRITE setShortcutDictate NOTIFY shortcutDictateChanged)
    Q_PROPERTY(QString shortcutCancel READ shortcutCancel WRITE setShortcutCancel NOTIFY shortcutCancelChanged)
    Q_PROPERTY(QString dictationMode READ dictationMode WRITE setDictationMode NOTIFY dictationModeChanged)
    Q_PROPERTY(QString outputMode READ outputMode WRITE setOutputMode NOTIFY outputModeChanged)
    Q_PROPERTY(bool clipboardFallback READ clipboardFallback WRITE setClipboardFallback NOTIFY clipboardFallbackChanged)
    Q_PROPERTY(QString activeModel READ activeModel WRITE setActiveModel NOTIFY activeModelChanged)
    Q_PROPERTY(QString defaultLanguage READ defaultLanguage WRITE setDefaultLanguage NOTIFY defaultLanguageChanged)
    Q_PROPERTY(QStringList enabledLanguages READ enabledLanguages WRITE setEnabledLanguages NOTIFY enabledLanguagesChanged)
    Q_PROPERTY(bool autoDetectLanguage READ autoDetectLanguage WRITE setAutoDetectLanguage NOTIFY autoDetectLanguageChanged)
    Q_PROPERTY(bool translateToEnglish READ translateToEnglish WRITE setTranslateToEnglish NOTIFY translateToEnglishChanged)
    Q_PROPERTY(QString backendMode READ backendMode WRITE setBackendMode NOTIFY backendModeChanged)
    Q_PROPERTY(QString backendApi READ backendApi WRITE setBackendApi NOTIFY backendApiChanged)
    Q_PROPERTY(int cpuThreads READ cpuThreads WRITE setCpuThreads NOTIFY cpuThreadsChanged)
    Q_PROPERTY(int silenceMs READ silenceMs WRITE setSilenceMs NOTIFY silenceMsChanged)
    Q_PROPERTY(int maxRecordingSeconds READ maxRecordingSeconds WRITE setMaxRecordingSeconds NOTIFY maxRecordingSecondsChanged)
    Q_PROPERTY(double vadThreshold READ vadThreshold WRITE setVadThreshold NOTIFY vadThresholdChanged)
    Q_PROPERTY(bool autoGainEnabled READ autoGainEnabled WRITE setAutoGainEnabled NOTIFY autoGainEnabledChanged)
    Q_PROPERTY(double inputGain READ inputGain WRITE setInputGain NOTIFY inputGainChanged)
    Q_PROPERTY(bool overlayEnabled READ overlayEnabled WRITE setOverlayEnabled NOTIFY overlayEnabledChanged)
    Q_PROPERTY(QString overlayPosition READ overlayPosition WRITE setOverlayPosition NOTIFY overlayPositionChanged)
    Q_PROPERTY(double overlayOpacity READ overlayOpacity WRITE setOverlayOpacity NOTIFY overlayOpacityChanged)
    Q_PROPERTY(double overlayScale READ overlayScale WRITE setOverlayScale NOTIFY overlayScaleChanged)
    Q_PROPERTY(bool overlayWaveform READ overlayWaveform WRITE setOverlayWaveform NOTIFY overlayWaveformChanged)
    Q_PROPERTY(bool overlaySounds READ overlaySounds WRITE setOverlaySounds NOTIFY overlaySoundsChanged)
    Q_PROPERTY(bool overlayReduceMotion READ overlayReduceMotion WRITE setOverlayReduceMotion NOTIFY overlayReduceMotionChanged)
    Q_PROPERTY(bool closeToTray READ closeToTray WRITE setCloseToTray NOTIFY closeToTrayChanged)
    Q_PROPERTY(bool trayIconAnimation READ trayIconAnimation WRITE setTrayIconAnimation NOTIFY trayIconAnimationChanged)
    Q_PROPERTY(bool cleanupEnabled READ cleanupEnabled WRITE setCleanupEnabled NOTIFY cleanupEnabledChanged)
    Q_PROPERTY(bool capitalizeSentences READ capitalizeSentences WRITE setCapitalizeSentences NOTIFY capitalizeSentencesChanged)
    Q_PROPERTY(bool removeFillerWords READ removeFillerWords WRITE setRemoveFillerWords NOTIFY removeFillerWordsChanged)
    Q_PROPERTY(bool addTrailingSpace READ addTrailingSpace WRITE setAddTrailingSpace NOTIFY addTrailingSpaceChanged)
    Q_PROPERTY(bool launchAtStartup READ launchAtStartup WRITE setLaunchAtStartup NOTIFY launchAtStartupChanged)
    Q_PROPERTY(bool startMinimized READ startMinimized WRITE setStartMinimized NOTIFY startMinimizedChanged)
    Q_PROPERTY(bool notificationsEnabled READ notificationsEnabled WRITE setNotificationsEnabled NOTIFY notificationsEnabledChanged)
    Q_PROPERTY(bool soundEffectsEnabled READ soundEffectsEnabled WRITE setSoundEffectsEnabled NOTIFY soundEffectsEnabledChanged)
    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged)
    Q_PROPERTY(bool telemetryEnabled READ telemetryEnabled WRITE setTelemetryEnabled NOTIFY telemetryEnabledChanged)
    Q_PROPERTY(bool storeRecordings READ storeRecordings WRITE setStoreRecordings NOTIFY storeRecordingsChanged)

    Q_PROPERTY(QString cleanupProvider READ cleanupProvider WRITE setCleanupProvider NOTIFY cleanupProviderChanged)
    Q_PROPERTY(QString cleanupLocalPreset READ cleanupLocalPreset WRITE setCleanupLocalPreset NOTIFY cleanupLocalPresetChanged)
    Q_PROPERTY(QString cleanupLocalEndpoint READ cleanupLocalEndpoint WRITE setCleanupLocalEndpoint NOTIFY cleanupLocalEndpointChanged)
    Q_PROPERTY(QString cleanupLocalModel READ cleanupLocalModel WRITE setCleanupLocalModel NOTIFY cleanupLocalModelChanged)
    Q_PROPERTY(QString cleanupRemoteProvider READ cleanupRemoteProvider WRITE setCleanupRemoteProvider NOTIFY cleanupRemoteProviderChanged)
    Q_PROPERTY(QString cleanupRemoteModel READ cleanupRemoteModel WRITE setCleanupRemoteModel NOTIFY cleanupRemoteModelChanged)
    Q_PROPERTY(QString cleanupRemoteEndpoint READ cleanupRemoteEndpoint WRITE setCleanupRemoteEndpoint NOTIFY cleanupRemoteEndpointChanged)
    Q_PROPERTY(QString cleanupSystemPrompt READ cleanupSystemPrompt WRITE setCleanupSystemPrompt NOTIFY cleanupSystemPromptChanged)
    Q_PROPERTY(bool cleanupUseVocab READ cleanupUseVocab WRITE setCleanupUseVocab NOTIFY cleanupUseVocabChanged)

    // Personal dictionary — JSON array of {from,to,caseSensitive,wholeWord,lang}.
    Q_PROPERTY(QString dictionary READ dictionary WRITE setDictionary NOTIFY dictionaryChanged)
    Q_PROPERTY(bool dictionaryBias READ dictionaryBias WRITE setDictionaryBias NOTIFY dictionaryBiasChanged)
    // Per-app output rules — JSON array of {match,mode}; gated by enabled flag.
    Q_PROPERTY(QString perAppRules READ perAppRules WRITE setPerAppRules NOTIFY perAppRulesChanged)
    Q_PROPERTY(bool perAppRulesEnabled READ perAppRulesEnabled WRITE setPerAppRulesEnabled NOTIFY perAppRulesEnabledChanged)

    // --- Profile module ---
    // Refine: text-shaping behaviors (feed rules where possible + the LLM always)
    Q_PROPERTY(bool refineSmartLists READ refineSmartLists WRITE setRefineSmartLists NOTIFY refineSmartListsChanged)
    Q_PROPERTY(bool refineBacktrack READ refineBacktrack WRITE setRefineBacktrack NOTIFY refineBacktrackChanged)
    Q_PROPERTY(bool refineAutoPunctuation READ refineAutoPunctuation WRITE setRefineAutoPunctuation NOTIFY refineAutoPunctuationChanged)
    // Personalize: voice templates (JSON [{cue,expansion}]) + tone preset
    Q_PROPERTY(QString voiceTemplates READ voiceTemplates WRITE setVoiceTemplates NOTIFY voiceTemplatesChanged)
    Q_PROPERTY(QString toneStyle READ toneStyle WRITE setToneStyle NOTIFY toneStyleChanged)
    // Devs Voice: dev mode + jargon vocabulary (newline/comma-separated terms)
    Q_PROPERTY(bool devModeEnabled READ devModeEnabled WRITE setDevModeEnabled NOTIFY devModeEnabledChanged)
    Q_PROPERTY(QString devJargon READ devJargon WRITE setDevJargon NOTIFY devJargonChanged)

public:
    explicit Settings(QObject* parent = nullptr);

    QString shortcutDictate() const;
    void setShortcutDictate(const QString& value);
    QString shortcutCancel() const;
    void setShortcutCancel(const QString& value);

    QString dictationMode() const;
    void setDictationMode(const QString& value);

    QString outputMode() const;
    void setOutputMode(const QString& value);
    bool clipboardFallback() const;
    void setClipboardFallback(bool value);

    QString activeModel() const;
    void setActiveModel(const QString& value);

    QString defaultLanguage() const;
    void setDefaultLanguage(const QString& value);
    QStringList enabledLanguages() const;
    void setEnabledLanguages(const QStringList& value);
    bool autoDetectLanguage() const;
    void setAutoDetectLanguage(bool value);
    bool translateToEnglish() const;
    void setTranslateToEnglish(bool value);

    QString backendMode() const;
    void setBackendMode(const QString& value);
    QString backendApi() const;
    void setBackendApi(const QString& value);
    int cpuThreads() const;
    void setCpuThreads(int value);

    int silenceMs() const;
    void setSilenceMs(int value);
    int maxRecordingSeconds() const;
    void setMaxRecordingSeconds(int value);
    double vadThreshold() const;
    void setVadThreshold(double value);
    bool autoGainEnabled() const;
    void setAutoGainEnabled(bool value);
    double inputGain() const;
    void setInputGain(double value);

    bool overlayEnabled() const;
    void setOverlayEnabled(bool value);
    QString overlayPosition() const;
    void setOverlayPosition(const QString& value);
    double overlayOpacity() const;
    void setOverlayOpacity(double value);
    double overlayScale() const;
    void setOverlayScale(double value);
    bool overlayWaveform() const;
    void setOverlayWaveform(bool value);
    bool overlaySounds() const;
    void setOverlaySounds(bool value);
    bool overlayReduceMotion() const;
    void setOverlayReduceMotion(bool value);
    bool closeToTray() const;
    void setCloseToTray(bool value);
    bool trayIconAnimation() const;
    void setTrayIconAnimation(bool value);

    bool cleanupEnabled() const;
    void setCleanupEnabled(bool value);
    bool capitalizeSentences() const;
    void setCapitalizeSentences(bool value);
    bool removeFillerWords() const;
    void setRemoveFillerWords(bool value);
    bool addTrailingSpace() const;
    void setAddTrailingSpace(bool value);

    bool launchAtStartup() const;
    void setLaunchAtStartup(bool value);
    bool startMinimized() const;
    void setStartMinimized(bool value);
    bool notificationsEnabled() const;
    void setNotificationsEnabled(bool value);
    bool soundEffectsEnabled() const;
    void setSoundEffectsEnabled(bool value);
    QString theme() const;
    void setTheme(const QString& value);

    bool telemetryEnabled() const;
    void setTelemetryEnabled(bool value);
    bool storeRecordings() const;
    void setStoreRecordings(bool value);

    QString cleanupProvider() const;
    void setCleanupProvider(const QString& value);
    QString cleanupLocalPreset() const;
    void setCleanupLocalPreset(const QString& value);
    QString cleanupLocalEndpoint() const;
    void setCleanupLocalEndpoint(const QString& value);
    QString cleanupLocalModel() const;
    void setCleanupLocalModel(const QString& value);
    QString cleanupRemoteProvider() const;
    void setCleanupRemoteProvider(const QString& value);
    QString cleanupRemoteModel() const;
    void setCleanupRemoteModel(const QString& value);
    QString cleanupRemoteEndpoint() const;
    void setCleanupRemoteEndpoint(const QString& value);
    QString cleanupSystemPrompt() const;
    void setCleanupSystemPrompt(const QString& value);
    bool cleanupUseVocab() const;
    void setCleanupUseVocab(bool value);

    QString dictionary() const;
    void setDictionary(const QString& value);
    bool dictionaryBias() const;
    void setDictionaryBias(bool value);
    QString perAppRules() const;
    void setPerAppRules(const QString& value);
    bool perAppRulesEnabled() const;
    void setPerAppRulesEnabled(bool value);

    bool refineSmartLists() const;
    void setRefineSmartLists(bool value);
    bool refineBacktrack() const;
    void setRefineBacktrack(bool value);
    bool refineAutoPunctuation() const;
    void setRefineAutoPunctuation(bool value);
    QString voiceTemplates() const;
    void setVoiceTemplates(const QString& value);
    QString toneStyle() const;
    void setToneStyle(const QString& value);
    bool devModeEnabled() const;
    void setDevModeEnabled(bool value);
    QString devJargon() const;
    void setDevJargon(const QString& value);

    Q_INVOKABLE void resetToDefaults();

signals:
    void shortcutDictateChanged();
    void shortcutCancelChanged();
    void dictationModeChanged();
    void outputModeChanged();
    void clipboardFallbackChanged();
    void activeModelChanged();
    void defaultLanguageChanged();
    void enabledLanguagesChanged();
    void autoDetectLanguageChanged();
    void translateToEnglishChanged();
    void backendModeChanged();
    void backendApiChanged();
    void cpuThreadsChanged();
    void silenceMsChanged();
    void maxRecordingSecondsChanged();
    void vadThresholdChanged();
    void autoGainEnabledChanged();
    void inputGainChanged();
    void overlayEnabledChanged();
    void overlayPositionChanged();
    void overlayOpacityChanged();
    void overlayScaleChanged();
    void overlayWaveformChanged();
    void overlaySoundsChanged();
    void overlayReduceMotionChanged();
    void closeToTrayChanged();
    void trayIconAnimationChanged();
    void cleanupEnabledChanged();
    void capitalizeSentencesChanged();
    void removeFillerWordsChanged();
    void addTrailingSpaceChanged();
    void launchAtStartupChanged();
    void startMinimizedChanged();
    void notificationsEnabledChanged();
    void soundEffectsEnabledChanged();
    void themeChanged();
    void telemetryEnabledChanged();
    void storeRecordingsChanged();
    void cleanupProviderChanged();
    void cleanupLocalPresetChanged();
    void cleanupLocalEndpointChanged();
    void cleanupLocalModelChanged();
    void cleanupRemoteProviderChanged();
    void cleanupRemoteModelChanged();
    void cleanupRemoteEndpointChanged();
    void cleanupSystemPromptChanged();
    void cleanupUseVocabChanged();
    void dictionaryChanged();
    void dictionaryBiasChanged();
    void perAppRulesChanged();
    void perAppRulesEnabledChanged();
    void refineSmartListsChanged();
    void refineBacktrackChanged();
    void refineAutoPunctuationChanged();
    void voiceTemplatesChanged();
    void toneStyleChanged();
    void devModeEnabledChanged();
    void devJargonChanged();

private:
    QSettings m_store;
};

} // namespace dictapulse
