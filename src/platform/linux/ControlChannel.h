// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#pragma once

#include <QObject>
#include <QString>

#include <functional>

class QLocalServer;

namespace dictapulse {

/// Single-instance control interface on a per-user local socket
/// (`$XDG_RUNTIME_DIR/dictapulse/control.sock`).
///
/// Compositors without a KGlobalAccel equivalent (Hyprland, sway, ...) bind keys
/// to a command such as `dictapulse --ptt-press`, which is delivered to the
/// running instance through this channel. One line in, one line out.
class ControlChannel : public QObject {
    Q_OBJECT
public:
    explicit ControlChannel(QObject* parent = nullptr);

    static QString socketPath();

    /// Commands the running instance understands.
    static bool isKnownCommand(const QString& command);

    /// Start listening. Returns false if another live instance owns the socket.
    bool listen();

    /// Provider for the reply to "status" (e.g. the current state name).
    void setStatusProvider(std::function<QString()> provider) { m_status = std::move(provider); }

    /// Client side: send one command to the running instance. Returns true if an
    /// instance answered. `reply` receives its answer line.
    static bool send(const QString& command, QString* reply = nullptr, int timeoutMs = 600);

signals:
    void toggleRequested();
    void pttPressed();
    void pttReleased();
    void cancelRequested();
    void showRequested();
    void quitRequested();

private:
    void onNewConnection();

    QLocalServer* m_server = nullptr;
    std::function<QString()> m_status;
};

} // namespace dictapulse
