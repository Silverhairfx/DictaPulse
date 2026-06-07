import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DictaPulse

ScrollView {
    id: pageRoot
    clip: true

    property var languageOptions: [
        { code: "en", name: qsTr("English") },
        { code: "ar", name: qsTr("Arabic") },
        { code: "it", name: qsTr("Italian") },
        { code: "fr", name: qsTr("French") },
        { code: "de", name: qsTr("German") },
        { code: "es", name: qsTr("Spanish") },
        { code: "pt", name: qsTr("Portuguese") },
        { code: "ru", name: qsTr("Russian") },
        { code: "tr", name: qsTr("Turkish") },
        { code: "nl", name: qsTr("Dutch") },
        { code: "pl", name: qsTr("Polish") },
        { code: "ja", name: qsTr("Japanese") },
        { code: "zh", name: qsTr("Chinese") }
    ]

    ColumnLayout {
        width: pageRoot.width
        spacing: Theme.gap

        SectionCard {
            title: qsTr("Speak in")
            subtitle: qsTr("Force the spoken language for accurate transcription. Auto-detect is best for clean English; on short or non-English clips it often guesses wrong, so set the language explicitly when speaking Arabic, Italian, etc.")

            RowLayout {
                Layout.fillWidth: true
                spacing: Theme.gap

                ClayComboBox {
                    id: langBox
                    Layout.preferredWidth: 200
                    enabled: !appSettings.autoDetectLanguage
                    textRole: "name"
                    valueRole: "code"
                    model: pageRoot.languageOptions
                    Component.onCompleted: {
                        const idx = indexOfValue(appSettings.defaultLanguage)
                        if (idx >= 0) currentIndex = idx
                    }
                    onActivated: appSettings.defaultLanguage = currentValue
                }

                ClayCheckBox {
                    text: qsTr("Auto-detect")
                    checked: appSettings.autoDetectLanguage
                    onToggled: appSettings.autoDetectLanguage = checked
                }

                ClayCheckBox {
                    text: qsTr("Translate to English")
                    checked: appSettings.translateToEnglish
                    onToggled: appSettings.translateToEnglish = checked
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Use Whisper's translate task — speak in any language, get English text.")
                    ToolTip.delay: 600
                }

                Item { Layout.fillWidth: true }

                StatusPill {
                    visible: controller.detectedLanguage !== ""
                    text: qsTr("Last detected: ") + controller.detectedLanguage.toUpperCase()
                    tint: Theme.accent
                }
            }
        }

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
                    color: Theme.bgWell
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
                ClayButton {
                    text: controller.state === "listening" ? qsTr("Stop") : qsTr("Start dictation")
                    variant: controller.state === "listening" ? "destructive" : "default"
                    onClicked: controller.toggleDictation()
                }
                ClayButton {
                    text: qsTr("Cancel")
                    variant: "outline"
                    enabled: controller.state === "listening" || controller.state === "processing"
                    onClicked: controller.cancelDictation()
                }
            }
        }

        SectionCard {
            title: qsTr("Last transcript")
            ClayTextArea {
                Layout.fillWidth: true
                Layout.preferredHeight: 120
                readOnly: true
                text: controller.lastTranscript
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
