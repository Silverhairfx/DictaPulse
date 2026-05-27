#include "Settings.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QThread>

namespace dictapulse {

namespace {
constexpr auto kShortcutDictate = "shortcuts/dictate";
constexpr auto kShortcutCancel = "shortcuts/cancel";
constexpr auto kDictationMode = "dictation/mode";
constexpr auto kOutputMode = "output/mode";
constexpr auto kClipboardFallback = "output/clipboardFallback";
constexpr auto kActiveModel = "models/active";
constexpr auto kDefaultLanguage = "language/default";
constexpr auto kEnabledLanguages = "language/enabled";
constexpr auto kAutoDetect = "language/autoDetect";
constexpr auto kTranslate = "language/translateToEnglish";
constexpr auto kBackendMode = "backend/mode";
constexpr auto kBackendApi = "backend/api";
constexpr auto kCpuThreads = "backend/cpuThreads";
constexpr auto kSilenceMs = "dictation/silenceMs";
constexpr auto kMaxSeconds = "dictation/maxSeconds";
constexpr auto kVadThreshold = "dictation/vadThreshold";
constexpr auto kAutoGain = "audio/autoGain";
constexpr auto kInputGain = "audio/inputGain";
constexpr auto kOverlayEnabled = "overlay/enabled";
// NOTE: avoid the literal "general/" group — QSettings reserves "General"
// (case-insensitively) for ungrouped keys and round-trips it asymmetrically in
// IniFormat (writes to [%General] but reads back the default). Use "app/".
constexpr auto kCloseToTray = "app/closeToTray";
constexpr auto kTrayAnimation = "app/trayAnimation";
constexpr auto kOverlayPosition = "overlay/position";
constexpr auto kOverlayOpacity = "overlay/opacity";
constexpr auto kOverlayScale = "overlay/scale";
constexpr auto kOverlayWaveform = "overlay/waveform";
constexpr auto kOverlaySounds = "overlay/sounds";
constexpr auto kOverlayReduceMotion = "overlay/reduceMotion";
constexpr auto kCleanupEnabled = "text/cleanup";
constexpr auto kCapitalize = "text/capitalize";
constexpr auto kRemoveFillers = "text/removeFillers";
constexpr auto kTrailingSpace = "text/trailingSpace";
constexpr auto kLaunchStartup = "app/launchAtStartup";
constexpr auto kStartMinimized = "app/startMinimized";
constexpr auto kNotifications = "app/notifications";
constexpr auto kSoundEffects = "app/soundEffects";
constexpr auto kTheme = "app/theme";
constexpr auto kTelemetry = "privacy/telemetry";
constexpr auto kStoreRecordings = "privacy/storeRecordings";
} // namespace

Settings::Settings(QObject* parent)
    : QObject(parent)
    , m_store(QSettings::IniFormat,
              QSettings::UserScope,
              QCoreApplication::organizationName(),
              QCoreApplication::applicationName())
{
}

#define DICTAPULSE_SETTING_STR(getter, setter, key, defaultValue, signal)                          \
    QString Settings::getter() const                                                               \
    {                                                                                              \
        return m_store.value(key, defaultValue).toString();                                        \
    }                                                                                              \
    void Settings::setter(const QString& value)                                                    \
    {                                                                                              \
        if (getter() == value) return;                                                             \
        m_store.setValue(key, value);                                                              \
        emit signal();                                                                             \
    }

#define DICTAPULSE_SETTING_BOOL(getter, setter, key, defaultValue, signal)                         \
    bool Settings::getter() const                                                                  \
    {                                                                                              \
        return m_store.value(key, defaultValue).toBool();                                          \
    }                                                                                              \
    void Settings::setter(bool value)                                                              \
    {                                                                                              \
        if (getter() == value) return;                                                             \
        m_store.setValue(key, value);                                                              \
        emit signal();                                                                             \
    }

#define DICTAPULSE_SETTING_INT(getter, setter, key, defaultValue, signal)                          \
    int Settings::getter() const                                                                   \
    {                                                                                              \
        return m_store.value(key, defaultValue).toInt();                                           \
    }                                                                                              \
    void Settings::setter(int value)                                                               \
    {                                                                                              \
        if (getter() == value) return;                                                             \
        m_store.setValue(key, value);                                                              \
        emit signal();                                                                             \
    }

#define DICTAPULSE_SETTING_DBL(getter, setter, key, defaultValue, signal)                          \
    double Settings::getter() const                                                                \
    {                                                                                              \
        return m_store.value(key, defaultValue).toDouble();                                        \
    }                                                                                              \
    void Settings::setter(double value)                                                            \
    {                                                                                              \
        if (qFuzzyCompare(getter(), value)) return;                                                \
        m_store.setValue(key, value);                                                              \
        emit signal();                                                                             \
    }

DICTAPULSE_SETTING_STR(shortcutDictate, setShortcutDictate, kShortcutDictate, "Ctrl+Alt+Space", shortcutDictateChanged)
DICTAPULSE_SETTING_STR(shortcutCancel, setShortcutCancel, kShortcutCancel, "Escape", shortcutCancelChanged)
DICTAPULSE_SETTING_STR(dictationMode, setDictationMode, kDictationMode, "toggle", dictationModeChanged)
DICTAPULSE_SETTING_STR(outputMode, setOutputMode, kOutputMode, "insert", outputModeChanged)
DICTAPULSE_SETTING_BOOL(clipboardFallback, setClipboardFallback, kClipboardFallback, true, clipboardFallbackChanged)
DICTAPULSE_SETTING_STR(activeModel, setActiveModel, kActiveModel, "", activeModelChanged)
DICTAPULSE_SETTING_STR(defaultLanguage, setDefaultLanguage, kDefaultLanguage, "en", defaultLanguageChanged)
DICTAPULSE_SETTING_BOOL(autoDetectLanguage, setAutoDetectLanguage, kAutoDetect, false, autoDetectLanguageChanged)
DICTAPULSE_SETTING_BOOL(translateToEnglish, setTranslateToEnglish, kTranslate, false, translateToEnglishChanged)
DICTAPULSE_SETTING_STR(backendMode, setBackendMode, kBackendMode, "gpu", backendModeChanged)
DICTAPULSE_SETTING_STR(backendApi, setBackendApi, kBackendApi, "vulkan", backendApiChanged)
DICTAPULSE_SETTING_INT(cpuThreads, setCpuThreads, kCpuThreads, qMax(1, QThread::idealThreadCount() / 2), cpuThreadsChanged)
DICTAPULSE_SETTING_INT(silenceMs, setSilenceMs, kSilenceMs, 1200, silenceMsChanged)
DICTAPULSE_SETTING_INT(maxRecordingSeconds, setMaxRecordingSeconds, kMaxSeconds, 60, maxRecordingSecondsChanged)
DICTAPULSE_SETTING_DBL(vadThreshold, setVadThreshold, kVadThreshold, 0.005, vadThresholdChanged)
DICTAPULSE_SETTING_BOOL(autoGainEnabled, setAutoGainEnabled, kAutoGain, true, autoGainEnabledChanged)
DICTAPULSE_SETTING_DBL(inputGain, setInputGain, kInputGain, 1.0, inputGainChanged)
DICTAPULSE_SETTING_BOOL(overlayEnabled, setOverlayEnabled, kOverlayEnabled, true, overlayEnabledChanged)
DICTAPULSE_SETTING_BOOL(closeToTray, setCloseToTray, kCloseToTray, true, closeToTrayChanged)
DICTAPULSE_SETTING_BOOL(trayIconAnimation, setTrayIconAnimation, kTrayAnimation, true, trayIconAnimationChanged)
DICTAPULSE_SETTING_STR(overlayPosition, setOverlayPosition, kOverlayPosition, "bottom-center", overlayPositionChanged)
DICTAPULSE_SETTING_DBL(overlayOpacity, setOverlayOpacity, kOverlayOpacity, 0.95, overlayOpacityChanged)
DICTAPULSE_SETTING_DBL(overlayScale, setOverlayScale, kOverlayScale, 0.8, overlayScaleChanged)
DICTAPULSE_SETTING_BOOL(overlayWaveform, setOverlayWaveform, kOverlayWaveform, true, overlayWaveformChanged)
DICTAPULSE_SETTING_BOOL(overlaySounds, setOverlaySounds, kOverlaySounds, true, overlaySoundsChanged)
DICTAPULSE_SETTING_BOOL(overlayReduceMotion, setOverlayReduceMotion, kOverlayReduceMotion, false, overlayReduceMotionChanged)
DICTAPULSE_SETTING_BOOL(cleanupEnabled, setCleanupEnabled, kCleanupEnabled, true, cleanupEnabledChanged)
DICTAPULSE_SETTING_BOOL(capitalizeSentences, setCapitalizeSentences, kCapitalize, true, capitalizeSentencesChanged)
DICTAPULSE_SETTING_BOOL(removeFillerWords, setRemoveFillerWords, kRemoveFillers, false, removeFillerWordsChanged)
DICTAPULSE_SETTING_BOOL(addTrailingSpace, setAddTrailingSpace, kTrailingSpace, true, addTrailingSpaceChanged)
DICTAPULSE_SETTING_BOOL(launchAtStartup, setLaunchAtStartup, kLaunchStartup, false, launchAtStartupChanged)
DICTAPULSE_SETTING_BOOL(startMinimized, setStartMinimized, kStartMinimized, false, startMinimizedChanged)
DICTAPULSE_SETTING_BOOL(notificationsEnabled, setNotificationsEnabled, kNotifications, true, notificationsEnabledChanged)
DICTAPULSE_SETTING_BOOL(soundEffectsEnabled, setSoundEffectsEnabled, kSoundEffects, false, soundEffectsEnabledChanged)
DICTAPULSE_SETTING_STR(theme, setTheme, kTheme, "system", themeChanged)
DICTAPULSE_SETTING_BOOL(telemetryEnabled, setTelemetryEnabled, kTelemetry, false, telemetryEnabledChanged)
DICTAPULSE_SETTING_BOOL(storeRecordings, setStoreRecordings, kStoreRecordings, false, storeRecordingsChanged)

QStringList Settings::enabledLanguages() const
{
    return m_store.value(kEnabledLanguages, QStringList { "en" }).toStringList();
}

void Settings::setEnabledLanguages(const QStringList& value)
{
    if (enabledLanguages() == value) return;
    m_store.setValue(kEnabledLanguages, value);
    emit enabledLanguagesChanged();
}

void Settings::resetToDefaults()
{
    m_store.clear();
    m_store.sync();
    emit shortcutDictateChanged();
    emit shortcutCancelChanged();
    emit dictationModeChanged();
    emit outputModeChanged();
    emit clipboardFallbackChanged();
    emit activeModelChanged();
    emit defaultLanguageChanged();
    emit enabledLanguagesChanged();
    emit autoDetectLanguageChanged();
    emit translateToEnglishChanged();
    emit backendModeChanged();
    emit backendApiChanged();
    emit cpuThreadsChanged();
    emit silenceMsChanged();
    emit maxRecordingSecondsChanged();
    emit vadThresholdChanged();
    emit autoGainEnabledChanged();
    emit inputGainChanged();
    emit overlayEnabledChanged();
    emit closeToTrayChanged();
    emit trayIconAnimationChanged();
    emit overlayPositionChanged();
    emit overlayOpacityChanged();
    emit overlayScaleChanged();
    emit overlayWaveformChanged();
    emit overlaySoundsChanged();
    emit overlayReduceMotionChanged();
    emit cleanupEnabledChanged();
    emit capitalizeSentencesChanged();
    emit removeFillerWordsChanged();
    emit addTrailingSpaceChanged();
    emit launchAtStartupChanged();
    emit startMinimizedChanged();
    emit notificationsEnabledChanged();
    emit soundEffectsEnabledChanged();
    emit themeChanged();
    emit telemetryEnabledChanged();
    emit storeRecordingsChanged();
}

} // namespace dictapulse
