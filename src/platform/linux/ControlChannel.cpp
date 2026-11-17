// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#include "ControlChannel.h"

#include <QDir>
#include <QFileInfo>
#include <QLocalServer>
#include <QLocalSocket>
#include <QStandardPaths>

namespace dictapulse {

namespace {
const QStringList& commands()
{
    static const QStringList v = {
        QStringLiteral("ping"),
        QStringLiteral("status"),
        QStringLiteral("toggle"),
        QStringLiteral("ptt-press"),
        QStringLiteral("ptt-release"),
        QStringLiteral("cancel"),
        QStringLiteral("show"),
        QStringLiteral("quit"),
    };
    return v;
}
} // namespace

ControlChannel::ControlChannel(QObject* parent)
    : QObject(parent)
{
}

QString ControlChannel::socketPath()
{
    // Override for running a second, isolated instance (automated tests).
    const QString override = qEnvironmentVariable("DICTAPULSE_CONTROL_SOCKET");
    if (!override.isEmpty()) return override;
    QString dir = qEnvironmentVariable("XDG_RUNTIME_DIR");
    if (dir.isEmpty()) dir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    return dir + QStringLiteral("/dictapulse/control.sock");
}

bool ControlChannel::isKnownCommand(const QString& command)
{
    return commands().contains(command);
}

bool ControlChannel::send(const QString& command, QString* reply, int timeoutMs)
{
    QLocalSocket sock;
    sock.connectToServer(socketPath());
    if (!sock.waitForConnected(timeoutMs)) return false;
    sock.write(command.toUtf8() + '\n');
    if (!sock.waitForBytesWritten(timeoutMs)) return false;
    if (!sock.waitForReadyRead(timeoutMs)) return false;
    const QByteArray line = sock.readLine().trimmed();
    if (reply) *reply = QString::fromUtf8(line);
    return line.startsWith("ok");
}

bool ControlChannel::listen()
{
    // A live instance answers "ping"; anything else is a stale socket file.
    if (send(QStringLiteral("ping"), nullptr, 300)) return false;

    const QString path = socketPath();
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile::setPermissions(QFileInfo(path).absolutePath(),
                          QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    QLocalServer::removeServer(path);

    m_server = new QLocalServer(this);
    m_server->setSocketOptions(QLocalServer::UserAccessOption);
    if (!m_server->listen(path)) return false;
    connect(m_server, &QLocalServer::newConnection, this, &ControlChannel::onNewConnection);
    return true;
}

void ControlChannel::onNewConnection()
{
    while (QLocalSocket* sock = m_server->nextPendingConnection()) {
        connect(sock, &QLocalSocket::disconnected, sock, &QObject::deleteLater);
        connect(sock, &QLocalSocket::readyRead, sock, [this, sock]() {
            while (sock->canReadLine()) {
                const QString cmd = QString::fromUtf8(sock->readLine()).trimmed();
                QByteArray reply = "ok\n";
                if (!isKnownCommand(cmd)) {
                    reply = "error unknown command\n";
                } else if (cmd == QLatin1String("status")) {
                    reply = "ok " + (m_status ? m_status() : QStringLiteral("unknown")).toUtf8() + "\n";
                } else if (cmd == QLatin1String("toggle")) {
                    emit toggleRequested();
                } else if (cmd == QLatin1String("ptt-press")) {
                    emit pttPressed();
                } else if (cmd == QLatin1String("ptt-release")) {
                    emit pttReleased();
                } else if (cmd == QLatin1String("cancel")) {
                    emit cancelRequested();
                } else if (cmd == QLatin1String("show")) {
                    emit showRequested();
                } else if (cmd == QLatin1String("quit")) {
                    emit quitRequested();
                }
                sock->write(reply);
                sock->flush();
            }
        });
    }
}

} // namespace dictapulse
