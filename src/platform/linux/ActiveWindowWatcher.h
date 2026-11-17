// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#pragma once

#include "HyprlandIpc.h"

#include <QObject>
#include <QString>

namespace dictapulse {

/// Tracks the focused window's application id / window class so per-app output
/// rules can react to "what am I typing into right now".
///
/// Hyprland: the focused window is read from Hyprland's IPC socket on demand.
///
/// Wayland (KDE): a client can't read the active window directly, so we load a
/// tiny KWin script ONCE that hooks window-activation and pushes the focused
/// window's resourceClass back to us over D-Bus. We cache it - zero cost at
/// dictation time. X11: queried on demand via xdotool.
class ActiveWindowWatcher : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.dictapulse.ActiveWindow")

public:
    explicit ActiveWindowWatcher(QObject* parent = nullptr);
    ~ActiveWindowWatcher() override;

    /// Lowercased app-id/class of the focused window, or empty if unknown.
    QString activeWindowId() const;

    /// Full identity of the focused window (Hyprland only; invalid elsewhere).
    HyprWindow activeWindow() const;
    bool usesHyprland() const { return m_hypr; }

public slots:
    /// Invoked by the KWin script over D-Bus on every window activation.
    Q_SCRIPTABLE void report(const QString& windowClass);

private:
    void setupWayland();
    QString queryX11() const;

    QString m_current;
    bool m_isWayland = false;
    bool m_hypr = false;
    int m_scriptId = -1;
};

} // namespace dictapulse
