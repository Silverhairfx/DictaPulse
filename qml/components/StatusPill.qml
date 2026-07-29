// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
import QtQuick
import QtQuick.Controls
import DictaPulse

// Tinted status chip with a glowing dot - sits flush on clay cards.
Rectangle {
    id: pill
    property string text: ""
    property color tint: Theme.accent
    implicitHeight: 22
    implicitWidth: row.implicitWidth + 18
    radius: height / 2
    color: Qt.rgba(tint.r, tint.g, tint.b, Theme.mode === "dark" ? 0.18 : 0.12)
    border.width: 1
    border.color: Qt.rgba(tint.r, tint.g, tint.b, 0.4)

    Row {
        id: row
        anchors.centerIn: parent
        spacing: 6
        Rectangle {
            width: 6; height: 6; radius: 3
            anchors.verticalCenter: parent.verticalCenter
            color: pill.tint
        }
        Label {
            text: pill.text
            color: Theme.mode === "dark"
                   ? Qt.lighter(pill.tint, 1.25)
                   : Qt.darker(pill.tint, 1.35)
            font.pixelSize: 11
            font.weight: Font.DemiBold
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
