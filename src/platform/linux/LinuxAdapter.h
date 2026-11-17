// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#pragma once

#include "HyprlandIpc.h"
#include "platform/PlatformAdapter.h"

#include <QPointer>
#include <QTimer>

class QAction;
class QIcon;
class QMenu;
class QSystemTrayIcon;

#ifdef DICTAPULSE_HAVE_KF6
class KStatusNotifierItem;
#endif

namespace dictapulse {

class ActiveWindowWatcher;
class ControlChannel;
class TextInjector;

/// Linux desktop adapter. The desktop is detected at runtime:
///  - KDE Plasma (when built with KF6): KGlobalAccel shortcuts, KStatusNotifierItem
///    tray, KNotification, KWin scripting for the active window.
///  - Hyprland and other Wayland compositors: a local control socket
///    (`dictapulse --toggle` / `--ptt-press` / `--ptt-release` / `--cancel`) that
///    compositor key binds call, a Qt StatusNotifierItem tray, and Hyprland IPC
///    for focus identity.
class LinuxAdapter : public PlatformAdapter {
    Q_OBJECT
public:
    explicit LinuxAdapter(QObject* parent = nullptr);
    ~LinuxAdapter() override;

    bool registerDictationShortcut(const QString& sequence) override;
    bool registerCancelShortcut(const QString& sequence) override;

    InjectResult injectText(const QString& text,
                            const QString& mode,
                            bool clipboardFallback) override;

    void showTrayMessage(const QString& title, const QString& body) override;
    void setTrayState(const QString& state, const QString& tooltip) override;
    QString activeWindowId() const override;
    void captureTarget() override;
    void setPressEnterAfterInsert(bool enabled) override { m_pressEnter = enabled; }

    void setAnimationEnabled(bool enabled);

    /// True when the KDE-specific paths (KGlobalAccel, KNotification) are in use.
    bool usesKde() const { return m_useKde; }

private:
    QAction* createGlobalAction(const QString& objectName,
                                const QString& label,
                                const QString& sequence);
    void buildTrayMenu();
    void buildGenericTray();
    void setTrayIconName(const QString& name);
    void startAnimation();
    void stopAnimation();
    void onAnimationTick();

    QAction* m_dictateAction = nullptr;
    QAction* m_cancelAction = nullptr;
    TextInjector* m_injector = nullptr;
    ActiveWindowWatcher* m_activeWindow = nullptr;
    ControlChannel* m_control = nullptr;
    HyprWindow m_target;
    QTimer m_animTimer;
    QString m_currentState = "idle";
    int m_animFrame = 0;
    bool m_animEnabled = true;
    bool m_pressEnter = false;
    bool m_useKde = false;
    QPointer<QSystemTrayIcon> m_sysTray;
    QMenu* m_sysMenu = nullptr;
#ifdef DICTAPULSE_HAVE_KF6
    KStatusNotifierItem* m_tray = nullptr;
#endif
};

} // namespace dictapulse
