// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#include "InsertionPolicy.h"

#include <QRegularExpression>

namespace dictapulse::insertion {

namespace {
const QStringList& terminalClasses()
{
    static const QStringList v = {
        QStringLiteral("kitty"),
        QStringLiteral("foot"),
        QStringLiteral("footclient"),
        QStringLiteral("alacritty"),
        QStringLiteral("wezterm"),
        QStringLiteral("org.wezfurlong.wezterm"),
        QStringLiteral("ghostty"),
        QStringLiteral("com.mitchellh.ghostty"),
        QStringLiteral("konsole"),
        QStringLiteral("org.kde.konsole"),
        QStringLiteral("xterm"),
        QStringLiteral("urxvt"),
        QStringLiteral("st"),
        QStringLiteral("st-256color"),
        QStringLiteral("gnome-terminal-server"),
        QStringLiteral("org.gnome.terminal"),
        QStringLiteral("terminator"),
        QStringLiteral("tilix"),
        QStringLiteral("xfce4-terminal"),
        QStringLiteral("lxterminal"),
        QStringLiteral("rio"),
        QStringLiteral("contour"),
        QStringLiteral("herdr-app"),
    };
    return v;
}
} // namespace

bool isTerminalClass(const QString& windowClass)
{
    return terminalClasses().contains(windowClass.trimmed().toLower());
}

bool isTerminalWindow(const HyprWindow& w)
{
    return isTerminalClass(w.cls) || isTerminalClass(w.comm);
}

PasteChord pasteChordFor(const QString& windowClass)
{
    return isTerminalClass(windowClass) ? PasteChord::CtrlShiftV : PasteChord::CtrlV;
}

PasteChord pasteChordFor(const HyprWindow& w)
{
    return isTerminalWindow(w) ? PasteChord::CtrlShiftV : PasteChord::CtrlV;
}

bool hasLineBreak(const QString& text)
{
    return text.contains(QLatin1Char('\n')) || text.contains(QLatin1Char('\r'))
        || text.contains(QChar(0x2028)) || text.contains(QChar(0x2029));
}

QString flattenLineBreaks(const QString& text)
{
    static const QRegularExpression re(QStringLiteral("[ \\t]*(?:\\r\\n|[\\r\\n\\x{2028}\\x{2029}])+[ \\t]*"));
    QString out = text;
    out.replace(re, QStringLiteral(" "));
    return out;
}

Decision decide(const Inputs& in)
{
    Decision d;
    d.text = in.text;
    d.chord = pasteChordFor(in.current.valid() ? in.current : in.target);

    // Focus moved between the start of dictation and now: never type into a
    // window the user did not dictate into. Keep the text on the clipboard.
    // An unknown target (nothing was focused, or the query failed) is treated the
    // same way: without proof of the destination, nothing is typed.
    if (!in.target.valid() || !in.current.valid() || in.current.address != in.target.address) {
        d.route = Route::Clipboard;
        d.focusChanged = true;
        return d;
    }

    if (in.mode == QLatin1String("clipboard")) {
        d.route = Route::Clipboard;
        return d;
    }

    const bool terminal = isTerminalWindow(in.current);
    const bool multiline = hasLineBreak(in.text);

    if (in.mode == QLatin1String("copy-paste")) {
        d.route = Route::Paste;
        // Pasting a line break into a terminal can execute a command.
        if (multiline && terminal) d.text = flattenLineBreaks(in.text);
    } else {
        // "insert" (and anything unknown): type through the virtual keyboard.
        if (multiline && !terminal) {
            // Multi-line text into a GUI app: paste keeps the line breaks
            // without sending any Enter key press.
            d.route = Route::Paste;
        } else {
            d.route = Route::Type;
            if (multiline) d.text = flattenLineBreaks(in.text);
        }
    }

    d.pressEnter = in.pressEnterAfter;
    return d;
}

QStringList wtypeTypeArgs(const QString& text)
{
    return { QStringLiteral("-d"), QStringLiteral("2"), QStringLiteral("--"), text };
}

QStringList wtypePasteArgs(PasteChord chord)
{
    if (chord == PasteChord::CtrlShiftV) {
        return { QStringLiteral("-M"), QStringLiteral("ctrl"), QStringLiteral("-M"), QStringLiteral("shift"),
                 QStringLiteral("v"),
                 QStringLiteral("-m"), QStringLiteral("shift"), QStringLiteral("-m"), QStringLiteral("ctrl") };
    }
    return { QStringLiteral("-M"), QStringLiteral("ctrl"), QStringLiteral("v"),
             QStringLiteral("-m"), QStringLiteral("ctrl") };
}

QStringList wtypeEnterArgs()
{
    return { QStringLiteral("-k"), QStringLiteral("Return") };
}

} // namespace dictapulse::insertion
