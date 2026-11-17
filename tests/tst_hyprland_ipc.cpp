// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#include "HyprlandIpc.h"

#include <QDir>
#include <QLocalServer>
#include <QLocalSocket>
#include <QTemporaryDir>
#include <QtTest>

using namespace dictapulse;

// Unix socket paths are limited to ~100 bytes, and a build or CI temp directory
// can already be longer than that. /dev/shm is short and not backed by disk.
static QString shortTempTemplate()
{
    return QFileInfo(QStringLiteral("/dev/shm")).isWritable()
        ? QStringLiteral("/dev/shm/dpt-XXXXXX")
        : QDir::temp().filePath(QStringLiteral("dpt-XXXXXX"));
}

static const char* kActiveWindow = R"({
  "address": "0x55803f7cc960", "mapped": true, "hidden": false,
  "at": [26, 50], "size": [3788, 1037],
  "workspace": {"id": 1, "name": "1"}, "floating": false, "monitor": 1,
  "class": "Kitty", "title": "~ : fish", "initialClass": "kitty",
  "pid": 754881, "xwayland": false
})";

static const char* kMonitors = R"([
  {"id": 0, "name": "DP-1", "x": 0, "y": 0, "width": 2560, "height": 1440, "scale": 1.0, "focused": false},
  {"id": 1, "name": "HDMI-A-2", "x": 2560, "y": 0, "width": 3840, "height": 2160, "scale": 1.5, "focused": true}
])";

class TstHyprlandIpc : public QObject {
    Q_OBJECT
private slots:
    void parseActiveWindow()
    {
        const HyprWindow w = HyprlandIpc::parseActiveWindow(kActiveWindow);
        QVERIFY(w.valid());
        QCOMPARE(w.address, QStringLiteral("0x55803f7cc960"));
        QCOMPARE(w.cls, QStringLiteral("kitty")); // lowercased
        QCOMPARE(w.title, QStringLiteral("~ : fish"));
        QCOMPARE(w.pid, qint64(754881));
        QCOMPARE(w.monitor, 1);
    }
    void parseNoWindow()
    {
        QVERIFY(!HyprlandIpc::parseActiveWindow("{}").valid());
        QVERIFY(!HyprlandIpc::parseActiveWindow("").valid());
        QVERIFY(!HyprlandIpc::parseActiveWindow("Invalid").valid());
    }
    void parseMonitors()
    {
        const auto m = HyprlandIpc::parseMonitors(kMonitors);
        QCOMPARE(m.size(), 2);
        QCOMPARE(m[1].name, QStringLiteral("HDMI-A-2"));
        QVERIFY(m[1].focused);
        QVERIFY(!m[0].focused);
        QCOMPARE(m[1].scale, 1.5);
        QVERIFY(HyprlandIpc::parseMonitors("nonsense").isEmpty());
    }

    void requestOverRealSocket()
    {
        // A stand-in Hyprland: one reply per connection, then close.
        QTemporaryDir runtime(shortTempTemplate());
        QVERIFY(runtime.isValid());
        const QString sig = QStringLiteral("testsig_123");
        QVERIFY(QDir().mkpath(runtime.path() + "/hypr/" + sig));
        const QString path = runtime.path() + "/hypr/" + sig + "/.socket.sock";

        QLocalServer server;
        QVERIFY(server.listen(path));
        QByteArray lastRequest;
        connect(&server, &QLocalServer::newConnection, this, [&]() {
            QLocalSocket* s = server.nextPendingConnection();
            connect(s, &QLocalSocket::readyRead, s, [&lastRequest, s]() {
                lastRequest = s->readAll();
                s->write(lastRequest == "j/monitors" ? kMonitors : kActiveWindow);
                s->flush();
                s->disconnectFromServer();
            });
        });

        qputenv("XDG_RUNTIME_DIR", runtime.path().toUtf8());
        qputenv("HYPRLAND_INSTANCE_SIGNATURE", sig.toUtf8());
        QVERIFY(HyprlandIpc::available());

        // request() blocks, so run it from a thread that lets the server answer.
        HyprWindow w;
        QVector<HyprMonitor> mons;
        QString focused;
        QThread* t = QThread::create([&]() {
            w = HyprlandIpc::activeWindow();
            mons = HyprlandIpc::monitors();
            focused = HyprlandIpc::focusedMonitorName();
        });
        t->start();
        QTRY_VERIFY_WITH_TIMEOUT(t->isFinished(), 5000);
        t->deleteLater();

        QCOMPARE(w.address, QStringLiteral("0x55803f7cc960"));
        QCOMPARE(mons.size(), 2);
        QCOMPARE(focused, QStringLiteral("HDMI-A-2"));
    }

    void unavailableWithoutSignature()
    {
        qunsetenv("HYPRLAND_INSTANCE_SIGNATURE");
        QVERIFY(!HyprlandIpc::available());
        QVERIFY(HyprlandIpc::request("j/activewindow").isEmpty());
        QVERIFY(!HyprlandIpc::activeWindow().valid());
    }
};

QTEST_GUILESS_MAIN(TstHyprlandIpc)
#include "tst_hyprland_ipc.moc"
