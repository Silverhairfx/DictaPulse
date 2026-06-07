// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
import QtQuick
import DictaPulse

Item {
    id: root
    property real level: 0.0
    property color color: Theme.accent
    property int bars: 28
    property bool reduceMotion: false

    implicitWidth: 220
    implicitHeight: 36

    QtObject {
        id: ampState
        property real smoothed: 0.0
        property var amps: []
        Component.onCompleted: {
            const a = []
            for (let i = 0; i < root.bars; ++i) a.push(0.05)
            amps = a
        }
    }

    Timer {
        interval: root.reduceMotion ? 80 : 33
        running: true
        repeat: true
        onTriggered: {
            // Higher gain (was 6) so normal speech drives the bars near full
            // height; light smoothing so peaks pop instead of being averaged out.
            const target = Math.min(1.0, root.level * 14.0)
            ampState.smoothed = ampState.smoothed * 0.55 + target * 0.45
            const a = ampState.amps.slice(1)
            const jitter = (Math.random() - 0.5) * 0.6
            a.push(Math.max(0.08, Math.min(1.0, ampState.smoothed + jitter * Math.max(0.25, ampState.smoothed))))
            ampState.amps = a
        }
    }

    Row {
        id: row
        anchors.fill: parent
        anchors.leftMargin: 2
        anchors.rightMargin: 2
        spacing: 3
        Repeater {
            model: root.bars
            delegate: Rectangle {
                required property int index
                width: (row.width - row.spacing * (root.bars - 1)) / root.bars
                // Use the full available height so the bars vibrate tall.
                height: Math.max(2, (ampState.amps[index] || 0.05) * row.height)
                anchors.verticalCenter: parent.verticalCenter
                radius: width / 2
                color: root.color
                opacity: 0.55 + 0.45 * (ampState.amps[index] || 0.05)
                Behavior on height { NumberAnimation { duration: root.reduceMotion ? 0 : 70 } }
            }
        }
    }
}
