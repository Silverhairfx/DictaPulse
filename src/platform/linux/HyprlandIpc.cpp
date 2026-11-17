// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#include "HyprlandIpc.h"

#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocalSocket>

namespace dictapulse {

QString HyprlandIpc::socketPath()
{
    const QByteArray sig = qgetenv("HYPRLAND_INSTANCE_SIGNATURE");
    if (sig.isEmpty()) return {};
    const QString runtime = qEnvironmentVariable("XDG_RUNTIME_DIR");
    const QString sigStr = QString::fromLocal8Bit(sig);
    if (!runtime.isEmpty()) {
        const QString p = runtime + QStringLiteral("/hypr/") + sigStr + QStringLiteral("/.socket.sock");
        if (QFileInfo::exists(p)) return p;
    }
    const QString legacy = QStringLiteral("/tmp/hypr/") + sigStr + QStringLiteral("/.socket.sock");
    return QFileInfo::exists(legacy) ? legacy : QString();
}

bool HyprlandIpc::available()
{
    return !socketPath().isEmpty();
}

QByteArray HyprlandIpc::request(const QByteArray& command, int timeoutMs)
{
    const QString path = socketPath();
    if (path.isEmpty()) return {};

    QLocalSocket sock;
    sock.connectToServer(path);
    if (!sock.waitForConnected(timeoutMs)) return {};
    sock.write(command);
    if (!sock.waitForBytesWritten(timeoutMs)) return {};

    QByteArray out;
    // Hyprland closes the connection after one reply.
    while (sock.state() != QLocalSocket::UnconnectedState) {
        if (!sock.waitForReadyRead(timeoutMs)) break;
        out += sock.readAll();
    }
    out += sock.readAll();
    return out;
}

HyprWindow HyprlandIpc::parseActiveWindow(const QByteArray& json)
{
    HyprWindow w;
    const QJsonDocument doc = QJsonDocument::fromJson(json);
    if (!doc.isObject()) return w;
    const QJsonObject o = doc.object();
    w.address = o.value(QStringLiteral("address")).toString();
    w.cls = o.value(QStringLiteral("class")).toString().trimmed().toLower();
    w.title = o.value(QStringLiteral("title")).toString();
    w.pid = static_cast<qint64>(o.value(QStringLiteral("pid")).toDouble(0));
    w.monitor = o.value(QStringLiteral("monitor")).toInt(-1);
    return w;
}

QVector<HyprMonitor> HyprlandIpc::parseMonitors(const QByteArray& json)
{
    QVector<HyprMonitor> out;
    const QJsonDocument doc = QJsonDocument::fromJson(json);
    if (!doc.isArray()) return out;
    for (const QJsonValue& v : doc.array()) {
        const QJsonObject o = v.toObject();
        HyprMonitor m;
        m.id = o.value(QStringLiteral("id")).toInt(-1);
        m.name = o.value(QStringLiteral("name")).toString();
        m.x = o.value(QStringLiteral("x")).toInt();
        m.y = o.value(QStringLiteral("y")).toInt();
        m.width = o.value(QStringLiteral("width")).toInt();
        m.height = o.value(QStringLiteral("height")).toInt();
        m.scale = o.value(QStringLiteral("scale")).toDouble(1.0);
        m.focused = o.value(QStringLiteral("focused")).toBool();
        if (!m.name.isEmpty()) out.push_back(m);
    }
    return out;
}

HyprWindow HyprlandIpc::activeWindow()
{
    return parseActiveWindow(request("j/activewindow"));
}

QVector<HyprMonitor> HyprlandIpc::monitors()
{
    return parseMonitors(request("j/monitors"));
}

QString HyprlandIpc::focusedMonitorName()
{
    for (const HyprMonitor& m : monitors()) {
        if (m.focused) return m.name;
    }
    return {};
}

} // namespace dictapulse
