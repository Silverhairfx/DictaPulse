// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#include "InsertionPolicy.h"

#include <QtTest>

using namespace dictapulse;
using namespace dictapulse::insertion;

namespace {
HyprWindow win(const QString& addr, const QString& cls)
{
    HyprWindow w;
    w.address = addr;
    w.cls = cls;
    return w;
}

Inputs inputs(const QString& text, const QString& mode, const HyprWindow& target, const HyprWindow& current)
{
    Inputs in;
    in.text = text;
    in.mode = mode;
    in.target = target;
    in.current = current;
    return in;
}
} // namespace

class TstInsertion : public QObject {
    Q_OBJECT
private slots:
    void chordSelection_data()
    {
        QTest::addColumn<QString>("cls");
        QTest::addColumn<bool>("terminalChord");
        QTest::newRow("kitty") << "kitty" << true;
        QTest::newRow("kitty upper") << "Kitty" << true;
        QTest::newRow("foot") << "foot" << true;
        QTest::newRow("alacritty") << "Alacritty" << true;
        QTest::newRow("wezterm") << "org.wezfurlong.wezterm" << true;
        QTest::newRow("konsole") << "org.kde.konsole" << true;
        QTest::newRow("herdr") << "herdr-app" << true;
        QTest::newRow("chrome") << "google-chrome" << false;
        QTest::newRow("slack") << "Slack" << false;
        QTest::newRow("claude") << "claude" << false;
        QTest::newRow("firefox") << "firefox" << false;
        QTest::newRow("empty") << "" << false;
        QTest::newRow("substring is not a terminal") << "skitty-game" << false;
    }
    void chordSelection()
    {
        QFETCH(QString, cls);
        QFETCH(bool, terminalChord);
        QCOMPARE(pasteChordFor(cls), terminalChord ? PasteChord::CtrlShiftV : PasteChord::CtrlV);
    }

    void chordArguments()
    {
        QCOMPARE(wtypePasteArgs(PasteChord::CtrlV),
                 (QStringList { "-M", "ctrl", "v", "-m", "ctrl" }));
        QCOMPARE(wtypePasteArgs(PasteChord::CtrlShiftV),
                 (QStringList { "-M", "ctrl", "-M", "shift", "v", "-m", "shift", "-m", "ctrl" }));
    }

    void focusChangeRefusesTyping()
    {
        const Decision d = decide(inputs("hello", "insert", win("0xA", "kitty"), win("0xB", "google-chrome")));
        QCOMPARE(d.route, Route::Clipboard);
        QVERIFY(d.focusChanged);
        QVERIFY(!d.pressEnter);
    }

    void focusChangeRefusesPasteToo()
    {
        const Decision d = decide(inputs("hello", "copy-paste", win("0xA", "kitty"), win("0xB", "kitty")));
        QCOMPARE(d.route, Route::Clipboard);
        QVERIFY(d.focusChanged);
    }

    void noFocusedWindowAtInsertRefuses()
    {
        const Decision d = decide(inputs("hello", "insert", win("0xA", "kitty"), HyprWindow()));
        QCOMPARE(d.route, Route::Clipboard);
        QVERIFY(d.focusChanged);
    }

    void unknownTargetRefuses()
    {
        // Without proof of where the text should go, nothing is typed.
        const Decision d = decide(inputs("hello", "insert", HyprWindow(), win("0xB", "kitty")));
        QCOMPARE(d.route, Route::Clipboard);
        QVERIFY(d.focusChanged);
    }

    void sameWindowTypes()
    {
        const Decision d = decide(inputs("hello", "insert", win("0xA", "kitty"), win("0xA", "kitty")));
        QCOMPARE(d.route, Route::Type);
        QVERIFY(!d.focusChanged);
        QCOMPARE(d.text, QStringLiteral("hello"));
    }

    void clipboardModeNeverTypes()
    {
        const Decision d = decide(inputs("hello", "clipboard", win("0xA", "kitty"), win("0xA", "kitty")));
        QCOMPARE(d.route, Route::Clipboard);
        QVERIFY(!d.focusChanged);
    }

    void copyPasteUsesTerminalChord()
    {
        const Decision d = decide(inputs("hello", "copy-paste", win("0xA", "kitty"), win("0xA", "kitty")));
        QCOMPARE(d.route, Route::Paste);
        QCOMPARE(d.chord, PasteChord::CtrlShiftV);
        const Decision g = decide(inputs("hello", "copy-paste", win("0xC", "slack"), win("0xC", "slack")));
        QCOMPARE(g.chord, PasteChord::CtrlV);
    }

    void arabicTextIsTypedUnchanged()
    {
        const QString arabic = QString::fromUtf8("مرحبا بالعالم، هذا اختبار hello world");
        const Decision d = decide(inputs(arabic, "insert", win("0xA", "kitty"), win("0xA", "kitty")));
        QCOMPARE(d.route, Route::Type);
        QCOMPARE(d.text, arabic);
        const QStringList args = wtypeTypeArgs(d.text);
        // The text reaches wtype as one UTF-8 argument after "--".
        QCOMPARE(args.last(), arabic);
        QCOMPARE(args.at(args.size() - 2), QStringLiteral("--"));
        QCOMPARE(QString::fromUtf8(args.last().toUtf8()), arabic);
        // No Latin-layout rewriting of any kind.
        QVERIFY(args.last().contains(QChar(0x0645)));
    }

    void arabicWithLineBreakInTerminalIsFlattened()
    {
        const QString text = QString::fromUtf8("السطر الأول\nالسطر الثاني");
        const Decision d = decide(inputs(text, "insert", win("0xA", "kitty"), win("0xA", "kitty")));
        QCOMPARE(d.route, Route::Type);
        QCOMPARE(d.text, QString::fromUtf8("السطر الأول السطر الثاني"));
    }

    void multilineInGuiAppIsPasted()
    {
        const Decision d = decide(inputs("one\ntwo", "insert", win("0xA", "slack"), win("0xA", "slack")));
        QCOMPARE(d.route, Route::Paste);
        QCOMPARE(d.text, QStringLiteral("one\ntwo"));
        QCOMPARE(d.chord, PasteChord::CtrlV);
    }

    void noEnterByDefault_data()
    {
        QTest::addColumn<QString>("text");
        QTest::addColumn<QString>("cls");
        QTest::newRow("single line terminal") << "ls -la" << "kitty";
        QTest::newRow("multi line terminal") << "echo a\necho b\n" << "kitty";
        QTest::newRow("trailing newline gui") << "send this\n" << "slack";
        QTest::newRow("carriage return") << "a\r\nb" << "foot";
    }
    void noEnterByDefault()
    {
        QFETCH(QString, text);
        QFETCH(QString, cls);
        const HyprWindow w = win("0xA", cls);
        const Decision d = decide(inputs(text, "insert", w, w));
        QVERIFY2(!d.pressEnter, "Enter must be off unless the setting is enabled");
        if (d.route == Route::Type) {
            QVERIFY(!hasLineBreak(d.text));
            const QStringList args = wtypeTypeArgs(d.text);
            QVERIFY(!args.contains(QStringLiteral("-k")));
        }
        // The paste chord never contains an Enter key either.
        for (PasteChord c : { PasteChord::CtrlV, PasteChord::CtrlShiftV })
            QVERIFY(!wtypePasteArgs(c).contains(QStringLiteral("Return")));
    }

    void enterOnlyWhenSettingEnabled()
    {
        const HyprWindow w = win("0xA", "kitty");
        Inputs in = inputs("echo hi", "insert", w, w);
        QVERIFY(!decide(in).pressEnter);
        in.pressEnterAfter = true;
        QVERIFY(decide(in).pressEnter);
        QCOMPARE(wtypeEnterArgs(), (QStringList { "-k", "Return" }));
        // Even then, a refused (focus changed) insertion never presses Enter.
        in.current = win("0xB", "kitty");
        QVERIFY(!decide(in).pressEnter);
    }

    void flattenKeepsSpacing()
    {
        QCOMPARE(flattenLineBreaks(QStringLiteral("a  \n  b")), QStringLiteral("a b"));
        QCOMPARE(flattenLineBreaks(QStringLiteral("a\n\nb")), QStringLiteral("a b"));
        QCOMPARE(flattenLineBreaks(QStringLiteral("plain")), QStringLiteral("plain"));
    }
};

QTEST_GUILESS_MAIN(TstInsertion)
#include "tst_insertion.moc"
