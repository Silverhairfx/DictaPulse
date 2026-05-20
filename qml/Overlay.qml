import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import DictaPulse

Window {
    id: overlayWin
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool | Qt.WA_TranslucentBackground
    color: "transparent"
    width: 360
    height: 60
    visible: false
    opacity: appSettings.overlayOpacity

    function showOverlay() {
        positionWindow()
        visible = true
        fadeIn.start()
    }
    function hideOverlay() {
        fadeOut.start()
    }
    function positionWindow() {
        const screen = Qt.application.screens[0]
        const sw = screen.geometry.width
        const sh = screen.geometry.height
        const pos = appSettings.overlayPosition
        let x = (sw - width) / 2
        let y = sh - height - 80
        if (pos === "top-center") y = 80
        if (pos === "cursor") {
            // best effort — Wayland will likely override
            x = Math.min(sw - width - 20, Math.max(20, sw / 2 - width / 2))
            y = Math.min(sh - height - 80, Math.max(80, sh / 2))
        }
        overlayWin.x = x + screen.geometry.x
        overlayWin.y = y + screen.geometry.y
    }

    NumberAnimation { id: fadeIn; target: overlayWin; property: "opacity"; from: 0.0; to: appSettings.overlayOpacity; duration: 160 }
    NumberAnimation {
        id: fadeOut
        target: overlayWin; property: "opacity"; to: 0.0; duration: 180
        onFinished: overlayWin.visible = false
    }

    Rectangle {
        id: pill
        anchors.fill: parent
        radius: 30
        color: Theme.overlayBg
        border.color: controller.state === "listening" ? Theme.accent
                     : controller.state === "processing" ? Theme.warning
                     : Theme.border
        border.width: 1

        layer.enabled: true
        layer.smooth: true

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            spacing: 10

            Rectangle {
                width: 10; height: 10; radius: 5
                color: controller.state === "listening" ? Theme.danger
                     : controller.state === "processing" ? Theme.warning
                     : Theme.textDim
                SequentialAnimation on opacity {
                    running: controller.state === "listening"
                    loops: Animation.Infinite
                    NumberAnimation { from: 1.0; to: 0.35; duration: 600 }
                    NumberAnimation { from: 0.35; to: 1.0; duration: 600 }
                }
            }

            Waveform {
                visible: appSettings.overlayWaveform
                Layout.fillWidth: true
                Layout.preferredHeight: 28
                level: controller.level
                color: Theme.accent
                reduceMotion: appSettings.overlayReduceMotion
            }

            Label {
                text: controller.state === "processing" ? qsTr("Transcribing…")
                    : controller.state === "listening" ? appSettings.defaultLanguage.toUpperCase()
                    : controller.state === "error" ? qsTr("Error")
                    : qsTr("Ready")
                color: Theme.textDim
                font.pixelSize: 11
                font.weight: Font.Medium
            }
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: controller.toggleDictation()
        }
    }
}
