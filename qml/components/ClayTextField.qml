import QtQuick
import QtQuick.Controls
import DictaPulse

// Clay text input: a well molded INTO the canvas; the rim lights up indigo
// on focus.
TextField {
    id: control

    implicitHeight: 36
    leftPadding: 12
    rightPadding: 12
    color: Theme.text
    placeholderTextColor: Theme.textDim
    selectionColor: Theme.accent
    selectedTextColor: Theme.onAccent
    font.pixelSize: 13
    verticalAlignment: TextInput.AlignVCenter

    background: ClaySurface {
        tier: "pressed"
        radius: Theme.radiusSm
        color: Theme.bgWell
        borderColor: control.activeFocus ? Theme.accent
                   : control.hovered ? Theme.borderHi : Theme.border
        borderWidth: control.activeFocus ? 1.5 : 1
        Behavior on borderColor { ColorAnimation { duration: 120 } }
    }
}
