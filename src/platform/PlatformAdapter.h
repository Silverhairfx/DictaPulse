#pragma once

#include <QObject>
#include <QString>

namespace dictapulse {

/// Abstract surface for desktop-environment-specific behavior. The Linux/KDE
/// adapter is the only one wired up today; future Windows/macOS/mobile shells
/// will subclass this so the core engine stays untouched.
class PlatformAdapter : public QObject {
    Q_OBJECT
public:
    enum class InjectResult {
        Inserted,
        ClipboardOnly,
        Failed,
    };

    explicit PlatformAdapter(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    virtual ~PlatformAdapter() = default;

    virtual bool registerDictationShortcut(const QString& sequence) = 0;
    virtual bool registerCancelShortcut(const QString& sequence) = 0;

    /// Inject text into the focused window/application.
    /// `mode`: "insert", "clipboard", "copy-paste"
    /// `clipboardFallback`: if insert mode fails, also copy to clipboard.
    virtual InjectResult injectText(const QString& text,
                                    const QString& mode,
                                    bool clipboardFallback)
        = 0;

    virtual void showTrayMessage(const QString& title, const QString& body) = 0;
    virtual void setTrayState(const QString& state, const QString& tooltip) = 0;

signals:
    void dictationShortcutPressed();
    void cancelShortcutPressed();
    void trayToggleRequested();
    void trayQuitRequested();
    void traySettingsRequested();
};

} // namespace dictapulse
