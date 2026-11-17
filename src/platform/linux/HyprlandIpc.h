// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#pragma once

#include <QByteArray>
#include <QString>
#include <QVector>

namespace dictapulse {

/// A window as reported by Hyprland's `j/activewindow`.
struct HyprWindow {
    QString address;  // stable per window while it exists, e.g. "0x55803f7cc960"
    QString cls;      // window class / app id, lowercased
    QString title;
    qint64 pid = 0;
    int monitor = -1;

    bool valid() const { return !address.isEmpty(); }
};

/// A monitor as reported by Hyprland's `j/monitors`.
struct HyprMonitor {
    int id = -1;
    QString name;
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    double scale = 1.0;
    bool focused = false;
};

/// Minimal client for Hyprland's request socket
/// (`$XDG_RUNTIME_DIR/hypr/$HYPRLAND_INSTANCE_SIGNATURE/.socket.sock`).
/// Read-only: it only issues `j/...` queries.
class HyprlandIpc {
public:
    /// True when running inside a Hyprland session whose socket is reachable.
    static bool available();
    static QString socketPath();

    /// Send one request and return the raw reply (empty on any failure).
    static QByteArray request(const QByteArray& command, int timeoutMs = 400);

    static HyprWindow activeWindow();
    static QVector<HyprMonitor> monitors();
    /// Name of the monitor that currently has focus, or empty.
    static QString focusedMonitorName();

    // Pure parsers (unit tested).
    static HyprWindow parseActiveWindow(const QByteArray& json);
    static QVector<HyprMonitor> parseMonitors(const QByteArray& json);
};

} // namespace dictapulse
