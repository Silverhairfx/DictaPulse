pragma Singleton

import QtQuick

QtObject {
    readonly property color bg:        "#0e1116"
    readonly property color bgRaised:  "#161b22"
    readonly property color bgHover:   "#1f2630"
    readonly property color border:    "#2a3340"
    readonly property color borderHi:  "#3a4555"
    readonly property color text:      "#e6edf3"
    readonly property color textDim:   "#8b97a8"
    readonly property color accent:    "#7c5cff"
    readonly property color accentSoft: Qt.rgba(0.486, 0.361, 1.0, 0.18)
    readonly property color success:   "#2ecc71"
    readonly property color warning:   "#f5a623"
    readonly property color danger:    "#ff5a5a"
    readonly property color overlayBg: Qt.rgba(0.055, 0.067, 0.086, 0.92)

    readonly property int   radius:    14
    readonly property int   radiusSm:  8
    readonly property int   pad:       16
    readonly property int   padSm:     10
    readonly property int   gap:       12
}
