import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DictaPulse

ScrollView {
    id: pageRoot
    clip: true

    ColumnLayout {
        width: pageRoot.width
        spacing: Theme.gap

        SectionCard {
            title: qsTr("Global shortcuts")
            subtitle: qsTr("Click a field, then press the key combination you want. Press Esc to cancel, Backspace to clear. Shortcuts apply instantly and work anywhere in KDE Plasma via KGlobalAccel.")

            SettingRow {
                label: qsTr("Start / stop dictation")
                hint: qsTr("Default: Ctrl+Alt+Space.")
                ShortcutCapture {
                    sequence: appSettings.shortcutDictate
                    onSequenceCaptured: function(seq) {
                        appSettings.shortcutDictate = seq
                        controller.applyShortcuts()
                    }
                    onCleared: {
                        appSettings.shortcutDictate = ""
                        controller.applyShortcuts()
                    }
                }
            }

            SettingRow {
                label: qsTr("Cancel dictation")
                hint: qsTr("Default: Escape (only works while the overlay is visible).")
                ShortcutCapture {
                    sequence: appSettings.shortcutCancel
                    onSequenceCaptured: function(seq) {
                        appSettings.shortcutCancel = seq
                        controller.applyShortcuts()
                    }
                    onCleared: {
                        appSettings.shortcutCancel = ""
                        controller.applyShortcuts()
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                ClayButton {
                    text: qsTr("Re-apply shortcuts")
                    variant: "secondary"
                    onClicked: controller.applyShortcuts()
                }
            }
        }

        SectionCard {
            title: qsTr("Dictation mode")
            SettingRow {
                label: qsTr("Mode")
                hint: qsTr("Push-to-talk holds while you speak. Toggle starts/stops with separate presses. Auto-stop ends after silence.")
                ClayComboBox {
                    width: 280
                    textRole: "label"
                    valueRole: "value"
                    model: [
                        { value: "toggle",    label: qsTr("Toggle (press to start / press to stop)") },
                        { value: "ptt",       label: qsTr("Push-to-talk (hold while speaking)") },
                        { value: "auto-stop", label: qsTr("Auto-stop on silence") }
                    ]
                    Component.onCompleted: {
                        const idx = indexOfValue(appSettings.dictationMode)
                        if (idx >= 0) currentIndex = idx
                    }
                    onActivated: appSettings.dictationMode = currentValue
                }
            }
        }
    }
}
