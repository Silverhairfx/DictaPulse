import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    width: 720
    height: 480
    minimumWidth: 480
    minimumHeight: 320
    visible: true
    title: qsTr("DictaPulse")

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 12

        Label {
            text: "DictaPulse"
            font.pixelSize: 36
            font.weight: Font.DemiBold
            Layout.alignment: Qt.AlignHCenter
        }

        Label {
            text: qsTr("Local AI voice dictation — scaffold ready.")
            opacity: 0.7
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
