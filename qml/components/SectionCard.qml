import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DictaPulse

Rectangle {
    id: card
    property string title: ""
    property string subtitle: ""
    default property alias content: contentColumn.data
    radius: Theme.radius
    color: Theme.bgRaised
    border.width: 1
    border.color: Theme.border
    Layout.fillWidth: true
    implicitHeight: header.implicitHeight + contentColumn.implicitHeight + Theme.pad * 3

    ColumnLayout {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: Theme.pad
        spacing: 2
        Label {
            text: card.title
            color: Theme.text
            font.pixelSize: 14
            font.weight: Font.DemiBold
            visible: text !== ""
        }
        Label {
            text: card.subtitle
            color: Theme.textDim
            font.pixelSize: 12
            visible: text !== ""
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }

    ColumnLayout {
        id: contentColumn
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.topMargin: Theme.gap
        anchors.leftMargin: Theme.pad
        anchors.rightMargin: Theme.pad
        anchors.bottomMargin: Theme.pad
        spacing: Theme.gap
    }
}
