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
            title: qsTr("Listening overlay")
            subtitle: qsTr("The small floating pill that appears when you start dictating.")

            SettingRow {
                label: qsTr("Show overlay while dictating")
                hint: qsTr("If off, no floating bar appears — rely on the tray icon for visual feedback.")
                Switch {
                    checked: appSettings.overlayEnabled
                    onToggled: appSettings.overlayEnabled = checked
                }
            }

            SettingRow {
                label: qsTr("Position")
                ComboBox {
                    width: 220
                    model: [
                        { value: "bottom-center", label: qsTr("Bottom center") },
                        { value: "top-center",    label: qsTr("Top center") },
                        { value: "cursor",        label: qsTr("Near cursor") }
                    ]
                    textRole: "label"
                    valueRole: "value"
                    Component.onCompleted: currentIndex = indexOfValue(appSettings.overlayPosition)
                    onActivated: appSettings.overlayPosition = currentValue
                }
            }

            SettingRow {
                label: qsTr("Opacity")
                RowLayout {
                    spacing: 8
                    Slider {
                        from: 0.5; to: 1.0; stepSize: 0.05
                        value: appSettings.overlayOpacity
                        onMoved: appSettings.overlayOpacity = value
                        Layout.preferredWidth: 160
                    }
                    Label { text: Math.round(appSettings.overlayOpacity * 100) + "%"; color: Theme.textDim }
                }
            }

            SettingRow {
                label: qsTr("Show waveform animation")
                Switch {
                    checked: appSettings.overlayWaveform
                    onToggled: appSettings.overlayWaveform = checked
                }
            }

            SettingRow {
                label: qsTr("Reduce motion")
                hint: qsTr("Slower, less animated waveform — easier on the eyes.")
                Switch {
                    checked: appSettings.overlayReduceMotion
                    onToggled: appSettings.overlayReduceMotion = checked
                }
            }
        }
    }
}
