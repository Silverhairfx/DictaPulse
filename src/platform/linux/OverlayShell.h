// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#pragma once

#include <QObject>
#include <QPointer>
#include <QString>

class QWindow;

namespace dictapulse {

/// Turns the listening overlay into a wlr layer-shell surface on Wayland
/// compositors that implement it (Hyprland, sway, ...). On KDE the overlay keeps
/// using the KWin window rule written by main(), so nothing changes there.
class OverlayShell : public QObject {
    Q_OBJECT
public:
    explicit OverlayShell(QObject* parent = nullptr);

    /// True when the overlay is managed as a layer-shell surface in this session.
    static bool wanted();

    /// Must run before the overlay window is first shown. Returns true when the
    /// window was configured as a layer-shell surface.
    Q_INVOKABLE bool attach(QWindow* window, const QString& position);

    /// Re-anchor (position setting changed) and move to the focused monitor.
    Q_INVOKABLE void reposition(const QString& position);
    Q_INVOKABLE void followFocusedMonitor();

private:
    QPointer<QWindow> m_window;
};

} // namespace dictapulse
