// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import QtMultimedia
import DictaPulse

Window {
    id: overlayWin
    objectName: "overlayWin"

    // Distinct title so the KWin rule (written by main.cpp) targets only this
    // window for bottom-center + keep-above placement.
    title: "DictaPulse Listening Overlay"

    // Detach from the main window: a transient child won't map on Wayland while
    // the parent is hidden (e.g. startMinimized). Top-level maps regardless.
    transientParent: null

    // Stays permanently mapped: Qt-on-Wayland can't re-map a QQuickWindow after
    // it's been hidden once, so the second show() never reappears. Input-transparent
    // so it never steals focus and clicks pass through to whatever's underneath.
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.WindowTransparentForInput
    color: "transparent"

    readonly property real baseWidth: 360
    readonly property real baseHeight: 60
    width: Math.round(baseWidth * appSettings.overlayScale)
    height: Math.round(baseHeight * appSettings.overlayScale)

    // Mapped whenever the feature is on; presence is driven by the pill's opacity,
    // not by mapping/unmapping the window.
    visible: appSettings.overlayEnabled
    opacity: appSettings.overlayOpacity

    readonly property bool active: controller.state === "listening"
                                 || controller.state === "processing"

    // Kept for Main.qml compat; show/hide is now just the opacity animation.
    function showOverlay() { raise() }
    function hideOverlay() {}

    SoundEffect {
        id: popIn
        source: "qrc:/qt/qml/DictaPulse/sounds/pop-in.wav"
        volume: 0.9
    }
    SoundEffect {
        id: popOut
        source: "qrc:/qt/qml/DictaPulse/sounds/pop-out.wav"
        volume: 0.9
    }

    // A suspended audio sink takes ~300ms to resume, so the first cue is dropped
    // mid-spin-up (why the start pop was unheard but the warm-sink end pop played).
    // Hold the sink awake with a real silent sample, then fire the cue once it's up.
    SoundEffect {
        id: sinkWarmup
        source: "qrc:/qt/qml/DictaPulse/sounds/silence.wav"
        volume: 1.0
    }
    Timer { id: startCueTimer; interval: 380; onTriggered: popIn.play() }
    Timer { id: endCueTimer; interval: 380; onTriggered: popOut.play() }

    // Pops signal listening/stopped as a non-visual cue. The sink can re-suspend
    // during a long recording, so warm it on both transitions, not just the start.
    onActiveChanged: {
        if (!appSettings.overlaySounds) return
        sinkWarmup.play()
        if (active) startCueTimer.restart()
        else        endCueTimer.restart()
    }

    Rectangle {
        id: pill
        anchors.fill: parent
        radius: height / 2
        color: Theme.overlayBg
        border.width: 1
        border.color: Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.45)

        // Fade content instead of mapping the window.
        opacity: overlayWin.active ? 1.0 : 0.0
        Behavior on opacity { NumberAnimation { duration: 160 } }

        readonly property real scale: appSettings.overlayScale

        // Clay sheen along the top edge.
        Rectangle {
            anchors.fill: parent
            anchors.margins: 1
            radius: height / 2
            gradient: Gradient {
                GradientStop { position: 0.0; color: Qt.rgba(1, 1, 1, 0.10) }
                GradientStop { position: 0.35; color: "transparent" }
                GradientStop { position: 1.0; color: Qt.rgba(0, 0, 0, 0.25) }
            }
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: Math.round(14 * pill.scale)
            anchors.rightMargin: Math.round(14 * pill.scale)
            spacing: Math.round(8 * pill.scale)

            // Red dot, pulses while listening.
            Rectangle {
                Layout.preferredWidth: Math.round(12 * pill.scale)
                Layout.preferredHeight: Math.round(12 * pill.scale)
                radius: width / 2
                color: "#ff3b30"
                SequentialAnimation on opacity {
                    running: controller.state === "listening"
                    loops: Animation.Infinite
                    NumberAnimation { from: 1.0; to: 0.35; duration: 600 }
                    NumberAnimation { from: 0.35; to: 1.0; duration: 600 }
                }
            }

            // Waveform; thin margins so bars span nearly the full pill height.
            Waveform {
                visible: appSettings.overlayWaveform
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.topMargin: Math.round(4 * pill.scale)
                Layout.bottomMargin: Math.round(4 * pill.scale)
                level: controller.level
                color: "white"
                reduceMotion: appSettings.overlayReduceMotion
            }
        }
    }
}
