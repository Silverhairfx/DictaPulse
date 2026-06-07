// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
import QtQuick
import QtQuick.Controls
import DictaPulse

// Clay progress bar: a pressed groove filling with indigo.
ProgressBar {
    id: control
    implicitHeight: 10

    background: ClaySurface {
        tier: "pressed"
        radius: height / 2
        color: Theme.bgWell
        borderColor: Theme.border
        borderWidth: 1
    }

    contentItem: Item {
        Rectangle {
            width: control.visualPosition * parent.width
            height: parent.height
            radius: height / 2
            color: Theme.accent
            Behavior on width { NumberAnimation { duration: 120 } }
        }
    }
}
