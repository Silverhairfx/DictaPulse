import QtQuick
import QtQuick.Controls
import DictaPulse

Item {
    id: root

    property string sequence: ""
    property string placeholder: qsTr("Click to set shortcut")
    signal sequenceCaptured(string seq)
    signal cleared()

    property bool capturing: false
    property string _liveText: ""

    implicitWidth: 220
    implicitHeight: 34

    activeFocusOnTab: true

    Rectangle {
        id: bg
        anchors.fill: parent
        radius: 6
        color: root.capturing ? Theme.accentSoft : Theme.bg
        border.color: root.capturing ? Theme.accent
                     : (root.activeFocus ? Theme.borderHi : Theme.border)
        border.width: 1

        Label {
            id: display
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: clearBtn.visible ? clearBtn.left : parent.right
            anchors.rightMargin: 6
            anchors.verticalCenter: parent.verticalCenter
            elide: Label.ElideRight
            font.pixelSize: 12
            font.family: "monospace"
            font.weight: Font.Medium
            text: root.capturing
                  ? (root._liveText !== "" ? root._liveText + "…" : qsTr("Press a key combination…"))
                  : (root.sequence !== "" ? root.sequence : root.placeholder)
            color: root.capturing ? Theme.accent
                  : (root.sequence !== "" ? Theme.text : Theme.textDim)
        }

        ToolButton {
            id: clearBtn
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: 2
            implicitWidth: 26
            implicitHeight: 26
            visible: root.sequence !== "" && !root.capturing
            text: "✕"
            font.pixelSize: 12
            onClicked: {
                root.sequence = ""
                root.cleared()
            }
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Clear shortcut")
            ToolTip.delay: 600
        }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        acceptedButtons: Qt.LeftButton
        onClicked: {
            if (!root.capturing) {
                root.capturing = true
                root._liveText = ""
                root.forceActiveFocus()
            }
        }
    }

    Keys.onPressed: function(event) {
        if (!root.capturing) return
        event.accepted = true

        if (event.key === Qt.Key_Escape) {
            root.capturing = false
            root._liveText = ""
            return
        }
        if (event.key === Qt.Key_Backspace) {
            root.sequence = ""
            root._liveText = ""
            root.capturing = false
            root.cleared()
            return
        }

        const modifierOnly = (event.key === Qt.Key_Control ||
                              event.key === Qt.Key_Alt ||
                              event.key === Qt.Key_AltGr ||
                              event.key === Qt.Key_Shift ||
                              event.key === Qt.Key_Meta)

        if (modifierOnly) {
            root._liveText = controller.modifierLabel(event.modifiers)
            return
        }

        const seq = controller.keySequenceFromEvent(event.key, event.modifiers)
        if (seq && seq.length > 0) {
            root.sequence = seq
            root.capturing = false
            root._liveText = ""
            root.sequenceCaptured(seq)
        }
    }

    onFocusChanged: if (!activeFocus && capturing) { capturing = false; _liveText = "" }
    onActiveFocusChanged: if (!activeFocus && capturing) { capturing = false; _liveText = "" }
}
