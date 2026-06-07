// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Layouts
import DictaPulse

import "profile"

// Profile module shell: an animated clay segmented tab bar over three sections.
// Everything here feeds the AI (Whisper vocabulary + cleanup-LLM system prompt)
// via ProfileContext on the C++ side.
Item {
    id: page
    property int tab: 0
    readonly property var tabs: [
        { label: qsTr("Refine"),      glyph: "✦" },
        { label: qsTr("Personalize"), glyph: "✸" },
        { label: qsTr("Devs Voice"),  glyph: "⌗" }
    ]

    ColumnLayout {
        anchors.fill: parent
        spacing: Theme.gap

        // --- Segmented tab bar ---
        Item {
            Layout.fillWidth: true
            implicitHeight: 46
            readonly property real slot: (width - 8) / 3

            ClaySurface {
                anchors.fill: parent
                tier: "pressed"
                radius: height / 2
                color: Theme.bgWell
                borderColor: Theme.border
                borderWidth: 1
            }

            // Glow behind the active pill.
            RectangularShadow {
                anchors.fill: indicator
                radius: indicator.radius
                blur: 22
                spread: 1
                color: Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.55)
                cached: false
            }
            ClaySurface {
                id: indicator
                width: parent.slot
                height: parent.height - 8
                y: 4
                x: 4 + page.tab * parent.slot
                tier: "sm"
                radius: height / 2
                color: Theme.accent
                embossStrength: 0.7
                borderColor: "transparent"
                borderWidth: 0
                Behavior on x { NumberAnimation { duration: 240; easing.type: Easing.OutBack; easing.overshoot: 1.1 } }
            }

            Row {
                anchors.fill: parent
                Repeater {
                    model: page.tabs
                    delegate: Item {
                        required property int index
                        required property var modelData
                        width: (parent.width - 8) / 3
                        height: parent.height
                        Row {
                            anchors.centerIn: parent
                            spacing: 7
                            Label {
                                text: modelData.glyph
                                anchors.verticalCenter: parent.verticalCenter
                                color: page.tab === index ? Theme.onAccent : Theme.textDim
                                font.pixelSize: 14
                            }
                            Label {
                                text: modelData.label
                                anchors.verticalCenter: parent.verticalCenter
                                color: page.tab === index ? Theme.onAccent : Theme.textDim
                                font.pixelSize: 13
                                font.weight: page.tab === index ? Font.DemiBold : Font.Medium
                            }
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: page.tab = index
                        }
                    }
                }
            }
        }

        // --- Content ---
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: page.tab

            ProfileRefine {}
            ProfilePersonalize {}
            ProfileDevsVoice {}
        }
    }
}
