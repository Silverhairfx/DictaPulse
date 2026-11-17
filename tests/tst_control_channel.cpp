// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#include "ControlChannel.h"

#include <QTemporaryDir>
#include <QThread>
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

class TstControlChannel : public QObject {
    Q_OBJECT
private:
    // The client API blocks, so it runs on a worker thread while the main thread
    // keeps the server's event loop turning.
    bool sendAsync(const QString& cmd, QString* reply = nullptr)
    {
        bool ok = false;
        QString r;
        QThread* t = QThread::create([&]() { ok = ControlChannel::send(cmd, &r, 2000); });
        t->start();
        while (!t->isFinished()) QTest::qWait(5);
        t->deleteLater();
        if (reply) *reply = r;
        return ok;
    }

private slots:
    void initTestCase()
    {
        static QTemporaryDir runtime(shortTempTemplate());
        QVERIFY(runtime.isValid());
        qputenv("XDG_RUNTIME_DIR", runtime.path().toUtf8());
    }

    void commandsAreRecognised()
    {
        for (const char* c : { "ping", "status", "toggle", "ptt-press", "ptt-release", "cancel", "show", "quit" })
            QVERIFY(ControlChannel::isKnownCommand(QString::fromLatin1(c)));
        QVERIFY(!ControlChannel::isKnownCommand(QStringLiteral("rm-rf")));
    }

    void noInstanceMeansSendFails()
    {
        QVERIFY(!ControlChannel::send(QStringLiteral("toggle"), nullptr, 300));
    }

    void deliversEveryCommandAndOnlyOnce()
    {
        ControlChannel ch;
        ch.setStatusProvider([]() { return QStringLiteral("listening"); });
        QVERIFY(ch.listen());

        QSignalSpy toggle(&ch, &ControlChannel::toggleRequested);
        QSignalSpy press(&ch, &ControlChannel::pttPressed);
        QSignalSpy release(&ch, &ControlChannel::pttReleased);
        QSignalSpy cancel(&ch, &ControlChannel::cancelRequested);
        QSignalSpy show(&ch, &ControlChannel::showRequested);

        QVERIFY(sendAsync("toggle"));
        QVERIFY(sendAsync("ptt-press"));
        QVERIFY(sendAsync("ptt-release"));
        QVERIFY(sendAsync("cancel"));
        QVERIFY(sendAsync("show"));
        QCOMPARE(toggle.count(), 1);
        QCOMPARE(press.count(), 1);
        QCOMPARE(release.count(), 1);
        QCOMPARE(cancel.count(), 1);
        QCOMPARE(show.count(), 1);

        QString reply;
        QVERIFY(sendAsync("status", &reply));
        QCOMPARE(reply, QStringLiteral("ok listening"));
        QVERIFY(sendAsync("ping"));
        QCOMPARE(toggle.count(), 1); // ping and status emit nothing
    }

    void unknownCommandIsRejected()
    {
        ControlChannel ch;
        QVERIFY(ch.listen());
        QSignalSpy toggle(&ch, &ControlChannel::toggleRequested);
        QString reply;
        QVERIFY(!sendAsync("toggle; reboot", &reply));
        QVERIFY(reply.startsWith(QStringLiteral("error")));
        QCOMPARE(toggle.count(), 0);
    }

    void secondInstanceCannotListen()
    {
        ControlChannel first;
        QVERIFY(first.listen());
        // listen() pings the socket, which needs the first channel's event loop.
        bool secondListened = true;
        QThread* t = QThread::create([&]() {
            ControlChannel second;
            secondListened = second.listen();
        });
        t->start();
        while (!t->isFinished()) QTest::qWait(5);
        t->deleteLater();
        QVERIFY(!secondListened);
    }

    void staleSocketIsReplaced()
    {
        {
            ControlChannel gone;
            QVERIFY(gone.listen());
        } // destroyed: the socket file may remain, nobody answers
        ControlChannel fresh;
        QVERIFY(fresh.listen());
    }
};

QTEST_GUILESS_MAIN(TstControlChannel)
#include "tst_control_channel.moc"
