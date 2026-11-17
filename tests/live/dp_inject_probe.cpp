// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
//
// Live insertion probe, used by scripts that run inside a disposable Hyprland
// instance. It is not part of ctest because it needs a compositor.
//
//   dp_inject_probe <insert|copy-paste|clipboard> <text> [--target ADDRESS] [--enter]
//
// The target defaults to the window that is focused when the probe starts, as
// the application captures it when dictation starts. Exit code: 0 inserted,
// 3 focus changed (text only copied), 4 clipboard only, 5 failed.
#include "TextInjector.h"

#include <QGuiApplication>
#include <QTimer>

#include <cstdio>

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    if (argc < 3) {
        std::fprintf(stderr, "usage: dp_inject_probe <mode> <text> [--target ADDRESS] [--enter]\n");
        return 2;
    }
    const QString mode = QString::fromLocal8Bit(argv[1]);
    const QString text = QString::fromUtf8(argv[2]);

    dictapulse::InsertTarget target;
    target.window = dictapulse::HyprlandIpc::activeWindow();
    for (int i = 3; i < argc; ++i) {
        const QString a = QString::fromLocal8Bit(argv[i]);
        if (a == QLatin1String("--target") && i + 1 < argc) target.window.address = QString::fromLocal8Bit(argv[++i]);
        else if (a == QLatin1String("--enter")) target.pressEnter = true;
    }

    dictapulse::TextInjector injector;
    const auto r = injector.inject(text, mode, true, target);
    int code = 5;
    switch (r) {
    case dictapulse::TextInjector::Result::Inserted: code = 0; break;
    case dictapulse::TextInjector::Result::FocusChanged: code = 3; break;
    case dictapulse::TextInjector::Result::ClipboardOnly: code = 4; break;
    case dictapulse::TextInjector::Result::Failed: code = 5; break;
    }
    std::printf("result=%d target=%s\n", code, qUtf8Printable(target.window.address));
    // Give the clipboard helper a moment before the process exits.
    QTimer::singleShot(200, &app, [code]() { QCoreApplication::exit(code); });
    return app.exec();
}
