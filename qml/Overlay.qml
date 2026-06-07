import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import QtMultimedia
import DictaPulse

Window {
    id: overlayWin
    objectName: "overlayWin"

    // Distinct title so a KWin window rule (written by main.cpp) can target ONLY
    // this window and force bottom-center + keep-above placement.
    title: "DictaPulse Listening Overlay"

    // Declared inside the main ApplicationWindow, this would default to being a
    // transient child — which on Wayland won't map while the parent is hidden
    // (e.g. when startMinimized hides the settings window). Detach it so the
    // overlay is an independent top-level that maps regardless of the main window.
    transientParent: null

    // WindowTransparentForInput is load-bearing: the window stays mapped at all
    // times (Qt-on-Wayland fails to *re-map* a QQuickWindow after it's been
    // hidden once — the second show() never reappears). Keeping it permanently
    // mapped sidesteps that bug; input-transparency means it never steals focus
    // and clicks pass straight through to whatever is underneath.
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.WindowTransparentForInput
    color: "transparent"

    readonly property real baseWidth: 360
    readonly property real baseHeight: 60
    width: Math.round(baseWidth * appSettings.overlayScale)
    height: Math.round(baseHeight * appSettings.overlayScale)

    // Mapped whenever the overlay feature is on; visual presence is driven by
    // the pill's opacity below, NOT by mapping/unmapping the window.
    visible: appSettings.overlayEnabled
    opacity: appSettings.overlayOpacity

    readonly property bool active: controller.state === "listening"
                                 || controller.state === "processing"

    // Kept for Main.qml compatibility; the pop sounds fire here, but show/hide
    // is now purely the opacity animation reacting to controller.state.
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

    // Play the pops as the overlay becomes/stops being active, so they signal
    // "now listening" / "stopped" — useful as a non-visual cue (accessibility).
    onActiveChanged: {
        if (!appSettings.overlaySounds) return
        if (active) popIn.play()
        else        popOut.play()
    }

    Rectangle {
        id: pill
        anchors.fill: parent
        radius: height / 2
        color: Theme.overlayBg
        border.width: 1
        border.color: Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.45)

        // Fade the content in/out instead of mapping the window.
        opacity: overlayWin.active ? 1.0 : 0.0
        Behavior on opacity { NumberAnimation { duration: 160 } }

        readonly property real scale: appSettings.overlayScale

        // Clay sheen: faint light along the top of the capsule.
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

            // Record button: solid red dot, pulses while listening.
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

            // White waveform, no text label. Minimal vertical margin so the
            // bars can vibrate nearly the full pill height.
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
