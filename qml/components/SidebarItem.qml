// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
import QtQuick
import QtQuick.Controls
import DictaPulse

// Sidebar nav entry: the active page becomes a raised indigo clay chip; the
// rest hover with a soft wash.
ItemDelegate {
    id: item
    property string label: ""
    property string emoji: ""
    property bool current: false
    implicitHeight: 38
    padding: 0
    hoverEnabled: true

    background: Item {
        ClaySurface {
            anchors.fill: parent
            visible: item.current
            tier: "sm"
            radius: Theme.radiusSm
            color: Theme.accent
            embossStrength: 0.7
            borderColor: "transparent"
            borderWidth: 0
        }
        Rectangle {
            anchors.fill: parent
            visible: !item.current && item.hovered
            radius: Theme.radiusSm
            color: Theme.bgHover
        }
    }

    contentItem: Row {
        spacing: 10
        leftPadding: 12
        rightPadding: 12
        Label {
            text: item.emoji
            anchors.verticalCenter: parent.verticalCenter
            color: item.current ? Theme.onAccent : Theme.textDim
            font.pixelSize: 14
        }
        Label {
            text: item.label
            color: item.current ? Theme.onAccent : (item.hovered ? Theme.text : Theme.textDim)
            font.pixelSize: 13
            font.weight: item.current ? Font.DemiBold : Font.Medium
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
