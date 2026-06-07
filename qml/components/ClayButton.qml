import QtQuick
import QtQuick.Controls
import DictaPulse

// Clay button with press physics: lifts 1px on hover, sinks and molds
// into the canvas on press. Derives from Button (not AbstractButton) so it
// can serve as a DialogButtonBox delegate.
Button {
    id: btn

    // "default" (indigo CTA) | "secondary" | "outline" | "ghost" | "destructive" | "success"
    property string variant: "default"
    property bool small: false

    readonly property color baseColor:
        variant === "default"     ? Theme.accent
      : variant === "destructive" ? Theme.danger
      : variant === "success"     ? Theme.success
      : variant === "secondary"   ? Theme.accentSoft
      : variant === "outline"     ? Theme.bgRaised
      : "transparent"

    readonly property color fgColor:
        variant === "default" ? Theme.onAccent
      : variant === "destructive" || variant === "success"
            ? (Qt.color(baseColor).hslLightness > 0.62 ? "#1a1208" : "#ffffff")
      : variant === "secondary" ? Theme.accentSoftFg
      : Theme.text

    implicitHeight: small ? 30 : 36
    implicitWidth: Math.max(label.implicitWidth + leftPadding + rightPadding,
                            small ? 64 : 84)
    // Own the padding — the base style's defaults would squeeze the label.
    padding: 0
    leftPadding: small ? 13 : 17
    rightPadding: small ? 13 : 17
    hoverEnabled: true
    focusPolicy: Qt.StrongFocus

    opacity: enabled ? 1.0 : 0.5

    background: Item {
        ClaySurface {
            id: face
            anchors.fill: parent
            visible: btn.variant !== "ghost"
            tier: btn.pressed ? "pressed" : "sm"
            radius: Theme.radiusSm
            embossStrength: btn.variant === "default" || btn.variant === "destructive"
                            || btn.variant === "success" ? 0.7 : 1.0
            color: btn.hovered && !btn.pressed
                   ? Qt.tint(btn.baseColor, Qt.rgba(1, 1, 1, Theme.mode === "dark" ? 0.07 : 0.0))
                   : btn.baseColor
            borderColor: btn.activeFocus ? Theme.accent
                       : btn.variant === "outline" || btn.variant === "secondary" ? Theme.border
                       : "transparent"
            borderWidth: btn.variant === "outline" || btn.variant === "secondary"
                         || btn.activeFocus ? 1 : 0
            // Press physics: -1px lift on hover, +1px sink on press.
            y: btn.pressed ? 1 : (btn.hovered ? -1 : 0)
            Behavior on y { NumberAnimation { duration: 90 } }
        }
        // Ghost: just a hover wash.
        Rectangle {
            anchors.fill: parent
            visible: btn.variant === "ghost" && (btn.hovered || btn.pressed)
            radius: Theme.radiusSm
            color: Theme.bgHover
            opacity: btn.pressed ? 1.0 : 0.7
        }
    }

    contentItem: Label {
        id: label
        text: btn.text
        color: btn.variant === "ghost" && btn.hovered ? Theme.text : btn.fgColor
        font.pixelSize: btn.small ? 12 : 13
        font.weight: Font.DemiBold
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        // transform (not y): the control owns contentItem's position.
        transform: Translate {
            id: lift
            y: btn.pressed ? 1 : (btn.hovered && btn.variant !== "ghost" ? -1 : 0)
            Behavior on y { NumberAnimation { duration: 90 } }
        }
    }
}
