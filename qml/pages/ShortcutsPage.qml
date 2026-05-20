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
            title: qsTr("Global shortcuts")
            subtitle: qsTr("These are registered with KGlobalAccel and work anywhere in KDE Plasma, even when DictaPulse is in the tray.")

            SettingRow {
                label: qsTr("Start / stop dictation")
                hint: qsTr("Default: Ctrl+Alt+Space. Click 'Apply' after changing.")
                TextField {
                    width: 200
                    text: appSettings.shortcutDictate
                    onEditingFinished: appSettings.shortcutDictate = text
                }
            }

            SettingRow {
                label: qsTr("Cancel dictation")
                TextField {
                    width: 200
                    text: appSettings.shortcutCancel
                    onEditingFinished: appSettings.shortcutCancel = text
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                Button {
                    text: qsTr("Apply shortcuts")
                    onClicked: controller.applyShortcuts()
                }
            }
        }

        SectionCard {
            title: qsTr("Dictation mode")
            SettingRow {
                label: qsTr("Mode")
                hint: qsTr("Push-to-talk holds while you speak. Toggle starts/stops with separate presses. Auto-stop ends after silence.")
                ComboBox {
                    width: 220
                    model: [
                        { value: "toggle", label: qsTr("Toggle (press to start / press to stop)") },
                        { value: "ptt", label: qsTr("Push-to-talk (hold while speaking)") },
                        { value: "auto-stop", label: qsTr("Auto-stop on silence") }
                    ]
                    textRole: "label"
                    valueRole: "value"
                    Component.onCompleted: currentIndex = indexOfValue(appSettings.dictationMode)
                    onActivated: appSettings.dictationMode = currentValue
                }
            }
        }
    }
}
