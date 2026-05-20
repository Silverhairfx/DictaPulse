import QtQuick
import DictaPulse

Item {
    id: root
    property real level: 0.0
    property color color: Theme.accent
    property int bars: 28
    property bool reduceMotion: false

    implicitWidth: 220
    implicitHeight: 36

    QtObject {
        id: ampState
        property real smoothed: 0.0
        property var amps: []
        Component.onCompleted: {
            const a = []
            for (let i = 0; i < root.bars; ++i) a.push(0.05)
            amps = a
        }
    }

    Timer {
        interval: root.reduceMotion ? 80 : 33
        running: true
        repeat: true
        onTriggered: {
            const target = Math.min(1.0, root.level * 6.0)
            ampState.smoothed = ampState.smoothed * 0.7 + target * 0.3
            const a = ampState.amps.slice(1)
            const jitter = (Math.random() - 0.5) * 0.45
            a.push(Math.max(0.06, Math.min(1.0, ampState.smoothed + jitter * Math.max(0.15, ampState.smoothed))))
            ampState.amps = a
        }
    }

    Row {
        id: row
        anchors.fill: parent
        anchors.leftMargin: 2
        anchors.rightMargin: 2
        spacing: 3
        Repeater {
            model: root.bars
            delegate: Rectangle {
                required property int index
                width: (row.width - row.spacing * (root.bars - 1)) / root.bars
                height: Math.max(2, (ampState.amps[index] || 0.05) * (row.height - 4))
                anchors.verticalCenter: parent.verticalCenter
                radius: width / 2
                color: root.color
                opacity: 0.5 + 0.5 * (ampState.amps[index] || 0.05)
                Behavior on height { NumberAnimation { duration: root.reduceMotion ? 0 : 80 } }
            }
        }
    }
}
