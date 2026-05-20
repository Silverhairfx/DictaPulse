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
            title: qsTr("Voice activity & silence")
            SettingRow {
                label: qsTr("Silence to auto-stop (ms)")
                hint: qsTr("How long silence must continue before DictaPulse stops listening. Only used in toggle and auto-stop modes.")
                SpinBox {
                    from: 200
                    to: 5000
                    stepSize: 100
                    value: appSettings.silenceMs
                    onValueModified: appSettings.silenceMs = value
                }
            }
            SettingRow {
                label: qsTr("Max recording length (s)")
                SpinBox {
                    from: 5
                    to: 600
                    value: appSettings.maxRecordingSeconds
                    onValueModified: appSettings.maxRecordingSeconds = value
                }
            }
            SettingRow {
                label: qsTr("Voice threshold (RMS)")
                hint: qsTr("Higher = needs louder speech to count as voice. Lower = picks up quieter input but more false positives.")
                Row {
                    spacing: 8
                    Slider {
                        from: 0.001
                        to: 0.05
                        stepSize: 0.001
                        value: appSettings.vadThreshold
                        onMoved: appSettings.vadThreshold = value
                        width: 160
                    }
                    Label {
                        text: appSettings.vadThreshold.toFixed(3)
                        color: Theme.textDim
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }
        }

        SectionCard {
            title: qsTr("General")
            SettingRow {
                label: qsTr("Launch on KDE startup")
                hint: qsTr("Installs an autostart .desktop entry in ~/.config/autostart/. (You can change this from System Settings → Autostart too.)")
                Switch {
                    checked: appSettings.launchAtStartup
                    onToggled: appSettings.launchAtStartup = checked
                }
            }
            SettingRow {
                label: qsTr("Start minimized to tray")
                Switch {
                    checked: appSettings.startMinimized
                    onToggled: appSettings.startMinimized = checked
                }
            }
            SettingRow {
                label: qsTr("Notifications")
                Switch {
                    checked: appSettings.notificationsEnabled
                    onToggled: appSettings.notificationsEnabled = checked
                }
            }
        }

        SectionCard {
            title: qsTr("Danger zone")
            RowLayout {
                Layout.fillWidth: true
                Button {
                    text: qsTr("Reset all settings")
                    onClicked: confirmReset.open()
                }
                Item { Layout.fillWidth: true }
            }
        }
    }

    Dialog {
        id: confirmReset
        title: qsTr("Reset all settings?")
        standardButtons: Dialog.Ok | Dialog.Cancel
        modal: true
        Label {
            text: qsTr("This clears every preference back to defaults. Downloaded models are not affected.")
            wrapMode: Text.Wrap
            width: 320
        }
        onAccepted: appSettings.resetToDefaults()
    }
}
