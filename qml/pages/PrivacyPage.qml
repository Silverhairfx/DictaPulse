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
            title: qsTr("Privacy")
            subtitle: qsTr("DictaPulse runs Whisper locally. Audio never leaves your device.")

            SettingRow {
                label: qsTr("Store recordings")
                hint: qsTr("If off (default), audio is held in memory only and discarded after transcription.")
                Switch {
                    checked: appSettings.storeRecordings
                    onToggled: appSettings.storeRecordings = checked
                }
            }

            SettingRow {
                label: qsTr("Anonymous telemetry")
                hint: qsTr("Off by default. No personal data is sent — only crash counts and feature usage. (Stub for now: nothing is collected.)")
                Switch {
                    checked: appSettings.telemetryEnabled
                    onToggled: appSettings.telemetryEnabled = checked
                }
            }
        }

        SectionCard {
            title: qsTr("Microphone")
            subtitle: qsTr("DictaPulse only records while the overlay is visible. KDE shows a microphone indicator in the status area whenever any app is using the mic — DictaPulse honors that system indicator.")
            Label {
                wrapMode: Text.Wrap
                Layout.fillWidth: true
                color: Theme.textDim
                font.pixelSize: 12
                text: qsTr("Tip: You can revoke microphone access at any time via System Settings → Audio → Recording.")
            }
        }
    }
}
