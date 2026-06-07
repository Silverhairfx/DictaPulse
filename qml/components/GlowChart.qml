// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
import QtQuick
import DictaPulse

// Neon-glow chart on a Canvas: bar or line, with a gradient area fill, a glowing
// stroke, and a left-to-right draw-in animation. Feed `values` (numbers) and
// optional `labels`.
Item {
    id: root
    property var values: []
    property var labels: []
    property color tint: Theme.accent
    property bool bars: false
    property int duration: 1100

    implicitHeight: 150

    // 0→1 reveal, animated whenever the data changes.
    property real reveal: 0
    NumberAnimation on reveal {
        id: revealAnim
        from: 0; to: 1
        duration: root.duration
        easing.type: Easing.OutCubic
        running: true
    }
    onValuesChanged: { reveal = 0; revealAnim.restart() }
    onRevealChanged: canvas.requestPaint()

    Connections {
        target: Theme
        function onChanged() { canvas.requestPaint() }
    }

    Canvas {
        id: canvas
        anchors.fill: parent
        renderStrategy: Canvas.Cooperative

        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()

        onPaint: {
            const ctx = getContext("2d")
            ctx.reset()
            ctx.clearRect(0, 0, width, height)

            const vals = root.values || []
            if (vals.length === 0) return

            let max = 0
            for (let i = 0; i < vals.length; ++i) max = Math.max(max, vals[i])
            if (max <= 0) max = 1

            const padX = 8
            const padTop = 12
            const padBottom = root.labels && root.labels.length ? 22 : 10
            const plotW = width - padX * 2
            const plotH = height - padTop - padBottom
            const baseY = padTop + plotH

            const c = root.tint
            const css = Qt.rgba(c.r, c.g, c.b, 1).toString()
            const glow = Qt.rgba(c.r, c.g, c.b, 0.9).toString()

            // Baseline grid line.
            ctx.strokeStyle = Theme.mode === "dark" ? "rgba(255,255,255,0.08)"
                                                    : "rgba(40,30,80,0.08)"
            ctx.lineWidth = 1
            ctx.beginPath(); ctx.moveTo(padX, baseY + 0.5); ctx.lineTo(width - padX, baseY + 0.5); ctx.stroke()

            const n = vals.length
            const rev = root.reveal

            function drawLabels() {
                if (!root.labels || !root.labels.length) return
                ctx.fillStyle = Theme.mode === "dark" ? "rgba(255,255,255,0.45)"
                                                      : "rgba(40,30,80,0.45)"
                ctx.font = "9px sans-serif"
                ctx.textAlign = "center"
                const step = Math.ceil(n / 7) // avoid clutter
                for (let i = 0; i < n; ++i) {
                    if (i % step !== 0 && i !== n - 1) continue
                    const x = bars ? padX + (i + 0.5) * (plotW / n)
                                   : padX + (n === 1 ? plotW / 2 : i * (plotW / (n - 1)))
                    ctx.fillText(String(root.labels[i]), x, height - 6)
                }
            }

            if (root.bars) {
                const slot = plotW / n
                const bw = Math.min(slot * 0.6, 34)
                for (let i = 0; i < n; ++i) {
                    const h = (vals[i] / max) * plotH * rev
                    const x = padX + i * slot + (slot - bw) / 2
                    const y = baseY - h
                    const g = ctx.createLinearGradient(0, y, 0, baseY)
                    g.addColorStop(0, css)
                    g.addColorStop(1, Qt.rgba(c.r, c.g, c.b, 0.15).toString())
                    ctx.fillStyle = g
                    ctx.shadowColor = glow
                    ctx.shadowBlur = 14
                    const r = Math.min(6, bw / 2)
                    ctx.beginPath()
                    ctx.moveTo(x, baseY)
                    ctx.lineTo(x, y + r)
                    ctx.arcTo(x, y, x + r, y, r)
                    ctx.lineTo(x + bw - r, y)
                    ctx.arcTo(x + bw, y, x + bw, y + r, r)
                    ctx.lineTo(x + bw, baseY)
                    ctx.closePath()
                    ctx.fill()
                    ctx.shadowBlur = 0
                }
                drawLabels()
            } else {
                // Area + glowing line, revealed left-to-right.
                const pts = []
                for (let i = 0; i < n; ++i) {
                    const x = padX + (n === 1 ? plotW / 2 : i * (plotW / (n - 1)))
                    const y = baseY - (vals[i] / max) * plotH
                    pts.push({ x: x, y: y })
                }
                const cutoff = padX + plotW * rev

                // Area fill.
                const g = ctx.createLinearGradient(0, padTop, 0, baseY)
                g.addColorStop(0, Qt.rgba(c.r, c.g, c.b, 0.35).toString())
                g.addColorStop(1, Qt.rgba(c.r, c.g, c.b, 0.0).toString())
                ctx.fillStyle = g
                ctx.beginPath()
                ctx.moveTo(pts[0].x, baseY)
                for (let i = 0; i < n; ++i) if (pts[i].x <= cutoff) ctx.lineTo(pts[i].x, pts[i].y)
                ctx.lineTo(Math.min(cutoff, pts[n - 1].x), baseY)
                ctx.closePath()
                ctx.fill()

                // Glowing stroke.
                ctx.strokeStyle = css
                ctx.lineWidth = 2.5
                ctx.lineJoin = "round"
                ctx.shadowColor = glow
                ctx.shadowBlur = 12
                ctx.beginPath()
                ctx.moveTo(pts[0].x, pts[0].y)
                for (let i = 1; i < n; ++i) if (pts[i].x <= cutoff) ctx.lineTo(pts[i].x, pts[i].y)
                ctx.stroke()
                ctx.shadowBlur = 0

                // End dot at the reveal frontier.
                let last = 0
                for (let i = 0; i < n; ++i) if (pts[i].x <= cutoff) last = i
                ctx.fillStyle = css
                ctx.shadowColor = glow
                ctx.shadowBlur = 10
                ctx.beginPath(); ctx.arc(pts[last].x, pts[last].y, 3.2, 0, Math.PI * 2); ctx.fill()
                ctx.shadowBlur = 0

                drawLabels()
            }
        }
    }
}
