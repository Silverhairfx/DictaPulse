// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
import QtQuick
import QtQuick.Controls
import DictaPulse

// Clay checkbox: a small pressed well that fills with indigo when checked.
CheckBox {
    id: control
    hoverEnabled: true
    spacing: 8
    opacity: enabled ? 1.0 : 0.5

    indicator: ClaySurface {
        implicitWidth: 20
        implicitHeight: 20
        x: control.leftPadding
        anchors.verticalCenter: parent.verticalCenter
        tier: control.checked ? "sm" : "pressed"
        radius: 6
        color: control.checked ? Theme.accent : Theme.bgWell
        borderColor: control.activeFocus ? Theme.accent
                   : control.hovered ? Theme.borderHi : Theme.border
        borderWidth: 1
        Behavior on color { ColorAnimation { duration: 120 } }

        Label {
            anchors.centerIn: parent
            text: "✓"
            color: Theme.onAccent
            font.pixelSize: 12
            font.weight: Font.Bold
            opacity: control.checked ? 1 : 0
            scale: control.checked ? 1 : 0.4
            Behavior on opacity { NumberAnimation { duration: 110 } }
            Behavior on scale { NumberAnimation { duration: 140; easing.type: Easing.OutBack } }
        }
    }

    contentItem: Label {
        text: control.text
        color: Theme.text
        font.pixelSize: 13
        verticalAlignment: Text.AlignVCenter
        leftPadding: control.indicator.width + control.spacing
        wrapMode: Text.WordWrap
    }
}
