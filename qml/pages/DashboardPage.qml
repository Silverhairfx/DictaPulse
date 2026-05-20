import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DictaPulse

ScrollView {
    clip: true
    ColumnLayout {
        width: parent.width
        spacing: Theme.gap

        SectionCard {
            title: qsTr("Status")
            subtitle: qsTr("Press your dictation shortcut anywhere to start speaking.")
            RowLayout {
                spacing: Theme.gap
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
                    color: Theme.text
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }
            }
            Label {
                visible: controller.lastError !== ""
                text: qsTr("Last error: ") + controller.lastError
                color: Theme.danger
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            // Live mic level — helpful when diagnosing 'no speech detected'.
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                visible: controller.state === "listening" || controller.level > 0.0005
                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        text: qsTr("Mic level")
                        color: Theme.textDim
                        font.pixelSize: 11
                    }
                    Item { Layout.fillWidth: true }
                    Label {
                        text: controller.level.toFixed(3) + "  (threshold " + appSettings.vadThreshold.toFixed(3) + ")"
                        color: controller.level > appSettings.vadThreshold ? Theme.success : Theme.textDim
                        font.pixelSize: 11
                        font.family: "monospace"
                    }
                }
                Rectangle {
                    Layout.fillWidth: true
                    height: 6
                    radius: 3
                    color: Theme.bg
                    border.color: Theme.border
                    Rectangle {
                        width: Math.min(parent.width, parent.width * Math.min(1.0, controller.level * 12))
                        height: parent.height
                        radius: 3
                        color: controller.level > appSettings.vadThreshold ? Theme.success : Theme.accent
                        Behavior on width { NumberAnimation { duration: 60 } }
                    }
                    Rectangle {
                        x: parent.width * Math.min(1.0, appSettings.vadThreshold * 12)
                        y: -2
                        width: 2
                        height: parent.height + 4
                        color: Theme.danger
                        opacity: 0.6
                    }
                }
            }

            RowLayout {
                spacing: Theme.gap
                Layout.fillWidth: true
                Button {
                    text: controller.state === "listening" ? qsTr("Stop") : qsTr("Start dictation")
                    onClicked: controller.toggleDictation()
                }
                Button {
                    text: qsTr("Cancel")
                    enabled: controller.state === "listening" || controller.state === "processing"
                    onClicked: controller.cancelDictation()
                }
            }
        }

        SectionCard {
            title: qsTr("Last transcript")
            TextArea {
                Layout.fillWidth: true
                Layout.preferredHeight: 120
                readOnly: true
                wrapMode: TextArea.Wrap
                text: controller.lastTranscript
                color: Theme.text
                background: Rectangle { color: Theme.bg; border.color: Theme.border; radius: 8 }
                placeholderText: qsTr("Your dictation will appear here after the first run.")
            }
        }

        SectionCard {
            title: qsTr("Quick info")
            GridLayout {
                columns: 2
                columnSpacing: Theme.pad
                rowSpacing: 6
                Layout.fillWidth: true
                Label { text: qsTr("Model"); color: Theme.textDim }
                Label { text: appSettings.activeModel || qsTr("none selected"); color: Theme.text }
                Label { text: qsTr("Backend"); color: Theme.textDim }
                Label { text: appSettings.backendMode + " · " + appSettings.backendApi; color: Theme.text }
                Label { text: qsTr("Language"); color: Theme.textDim }
                Label { text: appSettings.autoDetectLanguage ? qsTr("auto-detect") : appSettings.defaultLanguage; color: Theme.text }
                Label { text: qsTr("Output mode"); color: Theme.textDim }
                Label { text: appSettings.outputMode; color: Theme.text }
                Label { text: qsTr("Session"); color: Theme.textDim }
                Label { text: hardwareInfo.sessionType + " · " + hardwareInfo.desktopEnv; color: Theme.text }
            }
        }
    }
}
