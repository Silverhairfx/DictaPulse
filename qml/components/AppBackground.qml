import QtQuick
import DictaPulse

// Opaque app canvas with three large atmospheric radial washes (violet
// top-left, blue/purple top-right, mint bottom) — painted once per resize or
// theme flip, so it costs nothing per frame.
Item {
    id: root

    Rectangle {
        anchors.fill: parent
        color: Theme.bg
    }

    Canvas {
        id: washes
        anchors.fill: parent
        renderStrategy: Canvas.Cooperative

        onPaint: {
            const ctx = getContext("2d")
            ctx.reset()
            ctx.clearRect(0, 0, width, height)

            function wash(cx, cy, r, color) {
                const g = ctx.createRadialGradient(cx, cy, 0, cx, cy, r)
                g.addColorStop(0, color)
                g.addColorStop(1, Qt.rgba(color.r, color.g, color.b, 0))
                ctx.fillStyle = g
                ctx.fillRect(0, 0, width, height)
            }

            wash(width * 0.15, height * 0.10, Math.max(width, height) * 0.9, Theme.washA)
            wash(width * 0.85, 0,             Math.max(width, height) * 0.8, Theme.washB)
            wash(width * 0.50, height,        Math.max(width, height) * 0.8, Theme.washC)
        }

        Connections {
            target: Theme
            function onChanged() { washes.requestPaint() }
        }
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
    }
}
