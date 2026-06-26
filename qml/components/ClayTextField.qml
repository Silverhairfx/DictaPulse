// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
import QtQuick
import QtQuick.Controls
import DictaPulse

// Clay text input: a well in the canvas; rim lights up on focus.
TextField {
    id: control

    implicitHeight: 36
    leftPadding: 12
    rightPadding: 12
    // The font seats glyphs high in the line box, so plain AlignVCenter looks
    // lifted. Extra top padding drops the text to the optical centre.
    topPadding: 11
    bottomPadding: 5
    color: Theme.text
    placeholderTextColor: Theme.textDim
    selectionColor: Theme.accent
    selectedTextColor: Theme.onAccent
    font.pixelSize: 13
    verticalAlignment: TextInput.AlignVCenter

    background: ClaySurface {
        tier: "pressed"
        radius: Theme.radiusSm
        color: Theme.bgWell
        borderColor: control.activeFocus ? Theme.accent
                   : control.hovered ? Theme.borderHi : Theme.border
        borderWidth: control.activeFocus ? 1.5 : 1
        Behavior on borderColor { ColorAnimation { duration: 120 } }
    }
}
