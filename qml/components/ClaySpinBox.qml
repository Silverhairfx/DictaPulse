import QtQuick
import QtQuick.Controls
import DictaPulse

// Clay spinbox: a pressed well for the value with two raised − / + pebbles.
SpinBox {
    id: control
    implicitHeight: 36
    implicitWidth: 150
    editable: true
    hoverEnabled: true

    leftPadding: 36
    rightPadding: 36

    background: ClaySurface {
        tier: "pressed"
        radius: Theme.radiusSm
        color: Theme.bgWell
        borderColor: control.activeFocus ? Theme.accent : Theme.border
        borderWidth: 1
    }

    contentItem: TextInput {
        text: control.displayText
        color: Theme.text
        font.pixelSize: 13
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
        readOnly: !control.editable
        validator: control.validator
        inputMethodHints: Qt.ImhDigitsOnly
        selectionColor: Theme.accent
        selectedTextColor: Theme.onAccent
        clip: true
    }

    down.indicator: ClaySurface {
        x: 4
        anchors.verticalCenter: parent.verticalCenter
        width: 28
        height: 28
        tier: control.down.pressed ? "pressed" : "sm"
        radius: 8
        color: control.down.pressed ? Theme.bgWell
             : control.down.hovered ? Theme.bgHover : Theme.bgRaised
        borderColor: Theme.border
        borderWidth: 1
        Label {
            anchors.centerIn: parent
            text: "−"
            color: control.value > control.from ? Theme.text : Theme.textDim
            font.pixelSize: 15
            font.weight: Font.DemiBold
        }
    }

    up.indicator: ClaySurface {
        x: parent.width - width - 4
        anchors.verticalCenter: parent.verticalCenter
        width: 28
        height: 28
        tier: control.up.pressed ? "pressed" : "sm"
        radius: 8
        color: control.up.pressed ? Theme.bgWell
             : control.up.hovered ? Theme.bgHover : Theme.bgRaised
        borderColor: Theme.border
        borderWidth: 1
        Label {
            anchors.centerIn: parent
            text: "+"
            color: control.value < control.to ? Theme.text : Theme.textDim
            font.pixelSize: 15
            font.weight: Font.DemiBold
        }
    }
}
