// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
import QtQuick
import QtQuick.Effects
import DictaPulse

// The clay emboss engine ("tactile pop"): surfaces look molded out of the
// canvas - a soft sheen along the top, a pillow shade along the bottom, and a
// plush drop shadow underneath. Depth tiers:
//   "raised"  - cards & hero surfaces (clay)
//   "sm"      - buttons, chips, small controls (clay-sm)
//   "pressed" - inputs & active button state, molded INTO the canvas (clay-pressed)
//   "flat"    - no depth, just the rounded surface
Item {
    id: root

    property string tier: "raised"
    property color color: Theme.bgRaised
    property real radius: Theme.radius
    property color borderColor: Theme.border
    property real borderWidth: 1
    // Scales the inner sheen/shade - lets colored buttons keep a subtler gloss.
    property real embossStrength: 1.0

    default property alias contentData: surface.data

    readonly property bool _pressed: tier === "pressed"
    readonly property bool _raised: tier === "raised"

    // --- plush drop shadow (not for pressed/flat: those sit IN the canvas) ---
    RectangularShadow {
        anchors.fill: surface
        visible: !root._pressed && root.tier !== "flat"
        radius: root.radius
        blur: root._raised ? 22 : 10
        spread: 0
        offset.y: root._raised ? 7 : 3
        color: root._raised ? Theme.clayDrop : Theme.clayDropSoft
        cached: true
    }
    RectangularShadow {
        anchors.fill: surface
        visible: root._raised
        radius: root.radius
        blur: 6
        spread: 0
        offset.y: 2
        color: Theme.clayDropSoft
        cached: true
    }

    Rectangle {
        id: surface
        anchors.fill: parent
        radius: root.radius
        color: root.color
        border.color: root.borderColor
        border.width: root.borderWidth

        // Inset emboss. Raised: light sheen top → pillow shade bottom.
        // Pressed: inverted - shade pools at the top, faint light at the lip.
        Rectangle {
            anchors.fill: parent
            anchors.margins: root.borderWidth
            radius: Math.max(0, root.radius - root.borderWidth)
            gradient: Gradient {
                GradientStop {
                    position: 0.0
                    color: {
                        const c = root._pressed ? Theme.clayShade : Theme.clayHighlight
                        return Qt.rgba(c.r, c.g, c.b, c.a * (root._pressed ? 0.9 : 0.55) * root.embossStrength)
                    }
                }
                GradientStop { position: root._pressed ? 0.35 : 0.22; color: "transparent" }
                GradientStop { position: root._pressed ? 0.85 : 0.78; color: "transparent" }
                GradientStop {
                    position: 1.0
                    color: {
                        const c = root._pressed ? Theme.clayHighlight : Theme.clayShade
                        return Qt.rgba(c.r, c.g, c.b, c.a * (root._pressed ? 0.5 : 0.8) * root.embossStrength)
                    }
                }
            }
        }
    }
}
