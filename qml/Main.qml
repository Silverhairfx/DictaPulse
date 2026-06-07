// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Effects
import QtQuick.Layouts
import QtQuick.Window
import DictaPulse

ApplicationWindow {
    id: window
    objectName: "mainWin"
    width: 1000
    height: 660
    minimumWidth: 760
    minimumHeight: 520
    visible: !appSettings.startMinimized
    title: qsTr("DictaPulse")

    // Material is only the fallback chrome (popups, scrollbars); all visible
    // controls are the custom clay components.
    Material.theme: Theme.mode === "dark" ? Material.Dark : Material.Light
    Material.accent: Theme.accent
    Material.primary: Theme.accent
    Material.foreground: Theme.text
    Material.roundedScale: Material.SmallScale

    // Opaque clay canvas with atmospheric washes.
    background: AppBackground {}

    property int currentPage: devStartPage >= 0 && devStartPage < pages.length ? devStartPage : 0
    property var pages: [
        { label: qsTr("Dashboard"),     emoji: "◉", file: "pages/DashboardPage.qml" },
        { label: qsTr("Shortcuts"),     emoji: "⌘", file: "pages/ShortcutsPage.qml" },
        { label: qsTr("Models"),        emoji: "✦", file: "pages/ModelsPage.qml" },
        { label: qsTr("Language"),      emoji: "🌐", file: "pages/LanguagePage.qml" },
        { label: qsTr("Output"),        emoji: "↪", file: "pages/OutputPage.qml" },
        { label: qsTr("Cleanup"),       emoji: "✨", file: "pages/CleanupPage.qml" },
        { label: qsTr("Profile"),       emoji: "👤", file: "pages/ProfilePage.qml" },
        { label: qsTr("Backend"),       emoji: "⚙", file: "pages/BackendPage.qml" },
        { label: qsTr("Overlay"),       emoji: "◐", file: "pages/OverlayAppearancePage.qml" },
        { label: qsTr("Privacy"),       emoji: "🔒", file: "pages/PrivacyPage.qml" },
        { label: qsTr("Advanced"),      emoji: "⚡", file: "pages/AdvancedPage.qml" },
        { label: qsTr("About"),         emoji: "ⓘ", file: "pages/AboutPage.qml" }
    ]

    Connections {
        target: controller
        function onSettingsRequested() {
            window.show()
            window.raise()
            window.requestActivate()
        }
    }

    onClosing: function(closeEvent) {
        if (appSettings.closeToTray) {
            closeEvent.accepted = false
            window.hide()
        } else {
            controller.quitApp()
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        // --- Sidebar: a raised clay panel ---
        Item {
            Layout.preferredWidth: 228
            Layout.fillHeight: true

            ClaySurface {
                anchors.fill: parent
                tier: "raised"
                radius: Theme.radius + 2
                color: Theme.bgRaised
                borderColor: Theme.border
                borderWidth: 1
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 11

                    // Logo mark: the actual app icon (gradient tile + waveform),
                    // sitting on a clay drop shadow for depth.
                    Item {
                        implicitWidth: 36
                        implicitHeight: 36
                        RectangularShadow {
                            anchors.fill: logoImg
                            radius: 10
                            blur: 10
                            offset.y: 3
                            color: Theme.clayDrop
                            cached: true
                        }
                        Image {
                            id: logoImg
                            anchors.fill: parent
                            source: "qrc:/qt/qml/DictaPulse/icons/dictapulse.svg"
                            sourceSize: Qt.size(72, 72)
                            smooth: true
                        }
                    }
                    ColumnLayout {
                        spacing: 0
                        Layout.fillWidth: true
                        Label {
                            text: "DictaPulse"
                            color: Theme.text
                            font.family: Theme.displayFont
                            font.pixelSize: 18
                        }
                        Label {
                            text: qsTr("v0.1.0 · KDE Plasma")
                            color: Theme.textDim
                            font.pixelSize: 10
                        }
                    }

                    // Light / dark toggle: clay chip. The icon shows what a
                    // click switches *to*.
                    // Input lives on this wrapper Item, NOT inside ClaySurface —
                    // children routed through its default-property alias don't
                    // receive pointer events.
                    Item {
                        implicitWidth: 32
                        implicitHeight: 32

                        ClaySurface {
                            id: themeToggle
                            anchors.fill: parent
                            tier: themeTap.pressed ? "pressed" : "sm"
                            radius: 10
                            color: themeTap.containsMouse ? Theme.bgHover : Theme.bgRaised
                            borderColor: Theme.border
                            borderWidth: 1
                            Label {
                                anchors.centerIn: parent
                                text: Theme.mode === "dark" ? "☀" : "☾"
                                color: Theme.text
                                font.pixelSize: 15
                            }
                        }
                        MouseArea {
                            id: themeTap
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: appSettings.theme =
                                (Theme.mode === "dark" ? "light" : "dark")
                        }
                        ToolTip.visible: themeTap.containsMouse
                        ToolTip.text: Theme.mode === "dark"
                            ? qsTr("Switch to light") : qsTr("Switch to dark")
                    }
                }

                Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }

                Repeater {
                    model: window.pages
                    delegate: SidebarItem {
                        required property int index
                        required property var modelData
                        Layout.fillWidth: true
                        label: modelData.label
                        emoji: modelData.emoji
                        current: window.currentPage === index
                        onClicked: window.currentPage = index
                    }
                }

                Item { Layout.fillHeight: true }

                Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }

                RowLayout {
                    spacing: 8
                    Layout.fillWidth: true
                    StatusPill {
                        text: controller.state.toUpperCase()
                        tint: controller.state === "listening" ? Theme.success
                            : controller.state === "processing" ? Theme.accent
                            : controller.state === "error" ? Theme.danger
                            : Theme.textDim
                    }
                    Label {
                        text: controller.statusText
                        color: Theme.textDim
                        font.pixelSize: 11
                        elide: Label.ElideRight
                        Layout.fillWidth: true
                    }
                }
            }
        }

        // --- Content: serif page headline + the page itself ---
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10

            Label {
                id: pageTitle
                text: window.pages[window.currentPage].label
                color: Theme.text
                font.family: Theme.displayFont
                font.pixelSize: 26
                Layout.leftMargin: 4
            }

            Loader {
                id: pageLoader
                Layout.fillWidth: true
                Layout.fillHeight: true
                source: window.pages[window.currentPage].file
                opacity: status === Loader.Ready ? 1 : 0
                Behavior on opacity { NumberAnimation { duration: 150 } }
            }
        }
    }

    // Floating overlay lives as a sibling Window. Wayland refuses to display
    // Window items wrapped in a Loader inside another Window; keep it top-level.
    Overlay {
        id: overlay
    }

    Connections {
        target: controller
        function onOverlayRequested(show) {
            if (show) overlay.showOverlay()
            else      overlay.hideOverlay()
        }
    }
}
