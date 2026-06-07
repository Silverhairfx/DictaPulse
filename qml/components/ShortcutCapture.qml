import QtQuick
import QtQuick.Controls
import DictaPulse

// Shortcut recorder: a clay well that glows indigo while capturing.
Item {
    id: root

    property string sequence: ""
    property string placeholder: qsTr("Click to set shortcut")
    signal sequenceCaptured(string seq)
    signal cleared()

    property bool capturing: false
    property string _liveText: ""

    implicitWidth: 220
    implicitHeight: 36

    activeFocusOnTab: true

    ClaySurface {
        id: bg
        anchors.fill: parent
        tier: "pressed"
        radius: Theme.radiusSm
        color: root.capturing ? Theme.accentSoft : Theme.bgWell
        borderColor: root.capturing ? Theme.accent
                     : (root.activeFocus ? Theme.borderHi : Theme.border)
        borderWidth: root.capturing ? 1.5 : 1
        Behavior on color { ColorAnimation { duration: 120 } }

        Label {
            id: display
            anchors.left: parent.left
            anchors.leftMargin: 11
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
            color: root.capturing ? Theme.accentSoftFg
                  : (root.sequence !== "" ? Theme.text : Theme.textDim)
        }

        ToolButton {
            id: clearBtn
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: 3
            implicitWidth: 26
            implicitHeight: 26
            visible: root.sequence !== "" && !root.capturing
            text: "✕"
            font.pixelSize: 12
            background: Rectangle {
                radius: 7
                color: clearBtn.hovered ? Theme.bgHover : "transparent"
            }
            contentItem: Label {
                text: clearBtn.text
                color: Theme.textDim
                font.pixelSize: 12
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
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
        onClicked: function(mouse) {
            // Let the clear button keep working: ignore clicks on its rect.
            const p = mapToItem(clearBtn, mouse.x, mouse.y)
            if (clearBtn.visible && clearBtn.contains(Qt.point(p.x, p.y))) {
                clearBtn.clicked()
                return
            }
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
