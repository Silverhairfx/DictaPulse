// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#pragma once

#include "HyprlandIpc.h"

#include <QString>
#include <QStringList>

namespace dictapulse {

/// Pure decision logic for "where does dictated text go" on a compositor that
/// lets us identify the focused window. No I/O here, so it is unit tested.
namespace insertion {

enum class PasteChord {
    CtrlV,       // GUI applications
    CtrlShiftV,  // terminal emulators
};

enum class Route {
    Type,       // virtual keyboard types the text
    Paste,      // clipboard plus paste chord
    Clipboard,  // clipboard only, nothing is typed
};

struct Inputs {
    QString text;
    QString mode;                 // "insert", "copy-paste", "clipboard"
    HyprWindow target;            // focus captured when dictation started
    HyprWindow current;           // focus right now, at insertion time
    bool pressEnterAfter = false; // explicit opt-in only
};

struct Decision {
    Route route = Route::Clipboard;
    PasteChord chord = PasteChord::CtrlV;
    QString text;                 // text after newline handling
    bool focusChanged = false;    // refused to type because focus moved
    bool pressEnter = false;
};

/// Terminal emulators take ctrl+shift+v for paste, everything else ctrl+v.
bool isTerminalClass(const QString& windowClass);
/// Additional window classes to treat as terminals (user setting, lowercase
/// match), for terminal emulators the built-in list does not know.
void setExtraTerminalClasses(const QStringList& classes);
/// A window counts as a terminal by class or, for custom classes, by the name of
/// its process (a kitty started with --class scratch is still kitty).
bool isTerminalWindow(const HyprWindow& window);
PasteChord pasteChordFor(const QString& windowClass);
PasteChord pasteChordFor(const HyprWindow& window);

/// A newline typed through a virtual keyboard is an Enter key press, which
/// could submit a chat box or run a shell command. Typed text therefore never
/// contains line breaks: they become single spaces.
QString flattenLineBreaks(const QString& text);
bool hasLineBreak(const QString& text);

Decision decide(const Inputs& in);

/// Argument vectors for wtype (kept here so tests can assert on them).
QStringList wtypeTypeArgs(const QString& text);
QStringList wtypePasteArgs(PasteChord chord);
QStringList wtypeEnterArgs();

} // namespace insertion
} // namespace dictapulse
