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
            title: qsTr("Output mode")
            subtitle: qsTr("How transcribed text reaches your active app.")

            SettingRow {
                label: qsTr("Mode")
                ClayComboBox {
                    width: 260
                    model: [
                        { value: "insert",     label: qsTr("Direct insertion (default)") },
                        { value: "copy-paste", label: qsTr("Copy + paste (Ctrl+V)") },
                        { value: "clipboard",  label: qsTr("Clipboard only — never paste") }
                    ]
                    textRole: "label"
                    valueRole: "value"
                    Component.onCompleted: currentIndex = indexOfValue(appSettings.outputMode)
                    onActivated: appSettings.outputMode = currentValue
                }
            }

            SettingRow {
                label: qsTr("Fall back to clipboard if insertion fails")
                hint: qsTr("Recommended on Wayland — direct injection can fail in privileged windows.")
                ClaySwitch {
                    checked: appSettings.clipboardFallback
                    onToggled: appSettings.clipboardFallback = checked
                }
            }

            SettingRow {
                label: qsTr("Add trailing space")
                ClaySwitch {
                    checked: appSettings.addTrailingSpace
                    onToggled: appSettings.addTrailingSpace = checked
                }
            }
        }

        SectionCard {
            title: qsTr("Text cleanup")
            SettingRow {
                label: qsTr("Cleanup punctuation & spacing")
                ClaySwitch {
                    checked: appSettings.cleanupEnabled
                    onToggled: appSettings.cleanupEnabled = checked
                }
            }
            SettingRow {
                label: qsTr("Capitalize sentences")
                ClaySwitch {
                    checked: appSettings.capitalizeSentences
                    onToggled: appSettings.capitalizeSentences = checked
                }
            }
            SettingRow {
                label: qsTr("Remove filler words")
                hint: qsTr("Drops common English fillers: um, uh, erm, mm, hmm. English only.")
                ClaySwitch {
                    checked: appSettings.removeFillerWords
                    onToggled: appSettings.removeFillerWords = checked
                }
            }
        }
    }
}
