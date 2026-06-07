// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
import QtQuick
import QtQuick.Controls
import DictaPulse

// Clay slider: a pressed groove with an indigo fill and a raised pebble handle.
Slider {
    id: control
    hoverEnabled: true
    implicitHeight: 26

    background: Item {
        x: control.leftPadding
        y: control.topPadding + control.availableHeight / 2 - height / 2
        implicitWidth: 160
        implicitHeight: 8
        width: control.availableWidth

        ClaySurface {
            anchors.fill: parent
            tier: "pressed"
            radius: height / 2
            color: Theme.bgWell
            borderColor: Theme.border
            borderWidth: 1
        }
        Rectangle {
            width: control.visualPosition * parent.width
            height: parent.height
            radius: height / 2
            color: Theme.accent
        }
    }

    handle: ClaySurface {
        x: control.leftPadding + control.visualPosition * (control.availableWidth - width)
        y: control.topPadding + control.availableHeight / 2 - height / 2
        implicitWidth: 20
        implicitHeight: 20
        tier: "sm"
        radius: width / 2
        color: Theme.mode === "dark" ? "#e2e8f0" : "#ffffff"
        borderColor: control.activeFocus ? Theme.accent : Theme.border
        borderWidth: 1
        scale: control.pressed ? 1.12 : (control.hovered ? 1.06 : 1.0)
        Behavior on scale { NumberAnimation { duration: 90 } }
    }
}
