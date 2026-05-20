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
            title: qsTr("Speech-to-text models")
            subtitle: qsTr("Whisper GGML models from huggingface.co/ggerganov/whisper.cpp. Pick a smaller model for speed, larger for accuracy. Multilingual models support all of Whisper's languages including English, Arabic, and Italian.")

            ListView {
                id: list
                Layout.fillWidth: true
                Layout.preferredHeight: contentHeight
                interactive: false
                spacing: 8
                model: modelManager
                delegate: Rectangle {
                    width: list.width
                    radius: 10
                    color: Theme.bg
                    border.color: appSettings.activeModel === modelId ? Theme.accent : Theme.border
                    border.width: appSettings.activeModel === modelId ? 2 : 1
                    implicitHeight: layout.implicitHeight + 24

                    ColumnLayout {
                        id: layout
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 12
                        spacing: 8

                        RowLayout {
                            Layout.fillWidth: true
                            Label {
                                text: name
                                color: Theme.text
                                font.pixelSize: 14
                                font.weight: Font.DemiBold
                                Layout.fillWidth: true
                            }
                            StatusPill {
                                visible: appSettings.activeModel === modelId
                                text: qsTr("Active")
                                tint: Theme.accent
                            }
                            StatusPill {
                                visible: installed
                                text: qsTr("Installed")
                                tint: Theme.success
                            }
                            StatusPill {
                                visible: multilingual
                                text: qsTr("Multilingual")
                                tint: Theme.accent
                            }
                        }

                        RowLayout {
                            spacing: 16
                            Label { text: qsTr("Size: ") + sizeHuman; color: Theme.textDim; font.pixelSize: 11 }
                            Label { text: qsTr("Speed: ") + speed; color: Theme.textDim; font.pixelSize: 11 }
                            Label { text: qsTr("Accuracy: ") + accuracy; color: Theme.textDim; font.pixelSize: 11 }
                            Label { text: qsTr("Recommended RAM: ") + minRam; color: Theme.textDim; font.pixelSize: 11 }
                            Item { Layout.fillWidth: true }
                        }

                        ProgressBar {
                            visible: downloading
                            Layout.fillWidth: true
                            from: 0
                            to: 1
                            value: progress
                        }

                        RowLayout {
                            spacing: 8
                            Layout.fillWidth: true
                            Item { Layout.fillWidth: true }
                            Button {
                                visible: !installed && !downloading
                                text: qsTr("Download")
                                onClicked: modelManager.download(modelId)
                            }
                            Button {
                                visible: downloading
                                text: qsTr("Cancel")
                                onClicked: modelManager.cancel(modelId)
                            }
                            Button {
                                visible: installed && appSettings.activeModel !== modelId
                                text: qsTr("Use this model")
                                onClicked: appSettings.activeModel = modelId
                            }
                            Button {
                                visible: installed
                                text: qsTr("Delete")
                                onClicked: {
                                    if (appSettings.activeModel === modelId) appSettings.activeModel = ""
                                    modelManager.remove(modelId)
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
