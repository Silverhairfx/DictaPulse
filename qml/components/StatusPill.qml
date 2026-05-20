import QtQuick
import QtQuick.Controls
import DictaPulse

Rectangle {
    id: pill
    property string text: ""
    property color tint: Theme.accent
    implicitHeight: 22
    implicitWidth: row.implicitWidth + 16
    radius: height / 2
    color: Qt.rgba(tint.r, tint.g, tint.b, 0.15)
    border.width: 1
    border.color: Qt.rgba(tint.r, tint.g, tint.b, 0.45)

    Row {
        id: row
        anchors.centerIn: parent
        spacing: 6
        Rectangle {
            width: 6; height: 6; radius: 3
            anchors.verticalCenter: parent.verticalCenter
            color: pill.tint
        }
        Label {
            text: pill.text
            color: Theme.text
            font.pixelSize: 11
            font.weight: Font.Medium
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
