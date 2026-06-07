import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DictaPulse

ScrollView {
    clip: true
    ColumnLayout {
        width: parent.width
        spacing: Theme.gap

        SectionCard {
            title: qsTr("About DictaPulse")
            Label {
                color: Theme.text
                wrapMode: Text.Wrap
                Layout.fillWidth: true
                text: qsTr("DictaPulse is a local-first AI voice dictation tool for KDE Plasma.\nVersion 0.1.0 — free software under the GPL-3.0 license.")
            }
            Label {
                color: Theme.textDim
                wrapMode: Text.Wrap
                Layout.fillWidth: true
                font.pixelSize: 12
                text: qsTr("Built with Qt 6 Quick/QML, KDE Frameworks 6, and whisper.cpp. Cross-platform expansion to Windows, macOS, and mobile is planned.")
            }
            Label {
                color: Theme.textDim
                font.pixelSize: 12
                text: qsTr("Repository: https://github.com/Silverhairfx/DictaPulse")
            }
        }

        SectionCard {
            title: qsTr("Credits")
            Label {
                color: Theme.text
                font.pixelSize: 12
                wrapMode: Text.Wrap
                Layout.fillWidth: true
                text: qsTr("• Whisper models: ggerganov / whisper.cpp (MIT)\n• Tray: KStatusNotifierItem (KDE Frameworks)\n• Shortcuts: KGlobalAccel (KDE Frameworks)\n• Wayland text injection: wtype\n• Display font: DM Serif Display (SIL OFL 1.1)")
            }
        }
    }
}
