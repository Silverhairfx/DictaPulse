import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import DictaPulse

ApplicationWindow {
    id: window
    width: 980
    height: 640
    minimumWidth: 760
    minimumHeight: 520
    visible: !appSettings.startMinimized
    title: qsTr("DictaPulse")
    color: Theme.bg

    property int currentPage: 0
    property var pages: [
        { label: qsTr("Dashboard"),     emoji: "◉", file: "pages/DashboardPage.qml" },
        { label: qsTr("Shortcuts"),     emoji: "⌘", file: "pages/ShortcutsPage.qml" },
        { label: qsTr("Models"),        emoji: "✦", file: "pages/ModelsPage.qml" },
        { label: qsTr("Language"),      emoji: "🌐", file: "pages/LanguagePage.qml" },
        { label: qsTr("Output"),        emoji: "↪", file: "pages/OutputPage.qml" },
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
        // Hide to tray instead of quitting.
        closeEvent.accepted = false
        window.hide()
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 14

        // --- Sidebar ---
        Rectangle {
            Layout.preferredWidth: 220
            Layout.fillHeight: true
            color: Theme.bgRaised
            radius: Theme.radius
            border.color: Theme.border
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10
                    Rectangle {
                        width: 30; height: 30; radius: 8
                        color: Theme.accentSoft
                        border.color: Theme.accent
                        Label {
                            anchors.centerIn: parent
                            text: "◢"
                            color: Theme.accent
                            font.pixelSize: 16
                            font.weight: Font.Bold
                        }
                    }
                    ColumnLayout {
                        spacing: 0
                        Layout.fillWidth: true
                        Label {
                            text: "DictaPulse"
                            color: Theme.text
                            font.pixelSize: 16
                            font.weight: Font.DemiBold
                        }
                        Label {
                            text: qsTr("v0.1.0 · KDE Plasma")
                            color: Theme.textDim
                            font.pixelSize: 10
                        }
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

        // --- Content ---
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "transparent"

            Loader {
                id: pageLoader
                anchors.fill: parent
                source: window.pages[window.currentPage].file
            }
        }
    }

    // --- Floating overlay (separate Window) ---
    Loader {
        id: overlayLoader
        active: false
        sourceComponent: overlayComponent
    }

    Component {
        id: overlayComponent
        Overlay {}
    }

    Connections {
        target: controller
        function onOverlayRequested(show) {
            if (show) {
                overlayLoader.active = true
                if (overlayLoader.item) overlayLoader.item.showOverlay()
            } else if (overlayLoader.item) {
                overlayLoader.item.hideOverlay()
            }
        }
    }
}
