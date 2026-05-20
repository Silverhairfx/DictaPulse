import QtQuick
import QtQuick.Controls
import DictaPulse

ItemDelegate {
    id: item
    property string label: ""
    property string emoji: ""
    property bool current: false
    implicitHeight: 36
    padding: 0

    background: Rectangle {
        radius: 8
        color: item.current ? Theme.accentSoft : (item.hovered ? Theme.bgHover : "transparent")
    }

    contentItem: Row {
        spacing: 10
        leftPadding: 12
        rightPadding: 12
        Label {
            text: item.emoji
            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: 14
        }
        Label {
            text: item.label
            color: item.current ? Theme.text : Theme.textDim
            font.pixelSize: 13
            font.weight: item.current ? Font.DemiBold : Font.Normal
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
