// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
import QtQuick
import QtQuick.Controls
import DictaPulse

// Multi-line clay well (transcripts, prompts).
TextArea {
    id: control

    padding: 12
    color: Theme.text
    placeholderTextColor: Theme.textDim
    selectionColor: Theme.accent
    selectedTextColor: Theme.onAccent
    font.pixelSize: 13
    wrapMode: TextArea.Wrap

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
