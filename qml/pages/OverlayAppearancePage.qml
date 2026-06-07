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
                ClaySwitch {
                    checked: appSettings.overlayEnabled
                    onToggled: appSettings.overlayEnabled = checked
                }
            }

            SettingRow {
                label: qsTr("Position")
                ClayComboBox {
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
                    ClaySlider {
                        from: 0.5; to: 1.0; stepSize: 0.05
                        value: appSettings.overlayOpacity
                        onMoved: appSettings.overlayOpacity = value
                        Layout.preferredWidth: 160
                    }
                    Label { text: Math.round(appSettings.overlayOpacity * 100) + "%"; color: Theme.textDim }
                }
            }

            SettingRow {
                label: qsTr("Size")
                hint: qsTr("Adjust the overall size of the floating pill. Preview below.")
                ColumnLayout {
                    spacing: 10
                    RowLayout {
                        spacing: 8
                        ClaySlider {
                            id: scaleSlider
                            from: 0.5; to: 1.5; stepSize: 0.05
                            value: appSettings.overlayScale
                            onMoved: appSettings.overlayScale = value
                            Layout.preferredWidth: 200
                        }
                        Label {
                            text: Math.round(appSettings.overlayScale * 100) + "%"
                            color: Theme.textDim
                            Layout.preferredWidth: 48
                        }
                    }
                    // Live preview rendered at the actual size the overlay will use.
                    Rectangle {
                        Layout.preferredWidth: Math.round(360 * appSettings.overlayScale)
                        Layout.preferredHeight: Math.round(60 * appSettings.overlayScale)
                        radius: height / 2
                        color: Theme.overlayBg
                        border.color: Theme.accent
                        border.width: 1
                        opacity: appSettings.overlayOpacity
                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: Math.round(14 * appSettings.overlayScale)
                            anchors.rightMargin: Math.round(14 * appSettings.overlayScale)
                            spacing: Math.round(8 * appSettings.overlayScale)
                            Rectangle {
                                Layout.preferredWidth: Math.round(10 * appSettings.overlayScale)
                                Layout.preferredHeight: Math.round(10 * appSettings.overlayScale)
                                radius: width / 2
                                color: Theme.danger
                            }
                            Waveform {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                Layout.topMargin: Math.round(6 * appSettings.overlayScale)
                                Layout.bottomMargin: Math.round(6 * appSettings.overlayScale)
                                level: 0.45
                                color: Theme.accent
                            }
                            Label {
                                text: "AUTO"
                                color: Theme.textDim
                                font.pixelSize: Math.round(11 * appSettings.overlayScale)
                                font.weight: Font.Medium
                            }
                        }
                    }
                }
            }

            SettingRow {
                label: qsTr("Show waveform animation")
                ClaySwitch {
                    checked: appSettings.overlayWaveform
                    onToggled: appSettings.overlayWaveform = checked
                }
            }

            SettingRow {
                label: qsTr("Sound alerts")
                hint: qsTr("Play a rising tone when listening starts and a falling tone when it stops — an audible (and accessible) cue. Independent of the visual overlay.")
                ClaySwitch {
                    checked: appSettings.overlaySounds
                    onToggled: appSettings.overlaySounds = checked
                }
            }

            SettingRow {
                label: qsTr("Reduce motion")
                hint: qsTr("Slower, less animated waveform — easier on the eyes.")
                ClaySwitch {
                    checked: appSettings.overlayReduceMotion
                    onToggled: appSettings.overlayReduceMotion = checked
                }
            }
        }
    }
}
