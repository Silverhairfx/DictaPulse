// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
import QtQuick
import QtQuick.Controls
import DictaPulse

// Clay toggle: the track is a well pressed into the canvas; the thumb is a
// raised clay pebble that slides with a spring-ish ease and pops on hover.
Switch {
    id: control

    implicitWidth: 44
    implicitHeight: 26
    hoverEnabled: true
    opacity: enabled ? 1.0 : 0.5

    indicator: Item {
        implicitWidth: 44
        implicitHeight: 26
        anchors.verticalCenter: parent.verticalCenter

        ClaySurface {
            anchors.fill: parent
            tier: "pressed"
            radius: height / 2
            color: control.checked ? Theme.accent : Theme.bgWell
            borderColor: control.activeFocus ? Theme.accent : Theme.border
            borderWidth: 1
            Behavior on color { ColorAnimation { duration: 140 } }
        }

        ClaySurface {
            id: thumb
            width: 20
            height: 20
            anchors.verticalCenter: parent.verticalCenter
            x: control.checked ? parent.width - width - 3 : 3
            tier: "sm"
            radius: height / 2
            color: control.checked
                   ? (Theme.mode === "dark" ? Theme.onAccent : "#ffffff")
                   : (Theme.mode === "dark" ? "#cbd5e1" : "#ffffff")
            borderColor: "transparent"
            borderWidth: 0
            scale: control.pressed ? 0.92 : (control.hovered ? 1.06 : 1.0)
            Behavior on x { NumberAnimation { duration: 160; easing.type: Easing.OutBack; easing.overshoot: 1.2 } }
            Behavior on scale { NumberAnimation { duration: 90 } }
        }
    }

    contentItem: Label {
        text: control.text
        visible: control.text !== ""
        color: Theme.text
        font.pixelSize: 13
        verticalAlignment: Text.AlignVCenter
        leftPadding: control.indicator.width + 8
    }
}
