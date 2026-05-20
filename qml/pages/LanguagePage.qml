import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DictaPulse

ScrollView {
    clip: true
    ColumnLayout {
        width: parent.width
        spacing: Theme.gap

        property var languageOptions: [
            { code: "en", name: qsTr("English") },
            { code: "ar", name: qsTr("Arabic") },
            { code: "it", name: qsTr("Italian") },
            { code: "fr", name: qsTr("French") },
            { code: "de", name: qsTr("German") },
            { code: "es", name: qsTr("Spanish") },
            { code: "pt", name: qsTr("Portuguese") },
            { code: "ru", name: qsTr("Russian") },
            { code: "tr", name: qsTr("Turkish") },
            { code: "nl", name: qsTr("Dutch") },
            { code: "pl", name: qsTr("Polish") },
            { code: "ja", name: qsTr("Japanese") },
            { code: "zh", name: qsTr("Chinese") }
        ]

        SectionCard {
            title: qsTr("Default language")
            subtitle: qsTr("Speech is transcribed in this language unless auto-detect is on. English-only models (model name ends in .en) always use English regardless of this setting.")
            SettingRow {
                label: qsTr("Default")
                ComboBox {
                    width: 220
                    model: parent.parent.languageOptions
                    textRole: "name"
                    valueRole: "code"
                    Component.onCompleted: currentIndex = indexOfValue(appSettings.defaultLanguage)
                    onActivated: appSettings.defaultLanguage = currentValue
                }
            }
            SettingRow {
                label: qsTr("Auto-detect language")
                hint: qsTr("Whisper detects the language from the first ~30s. Slightly slower; only works on multilingual models.")
                Switch {
                    checked: appSettings.autoDetectLanguage
                    onToggled: appSettings.autoDetectLanguage = checked
                }
            }
        }

        SectionCard {
            title: qsTr("Enabled languages")
            subtitle: qsTr("Used as hints for auto-detect priority and for future per-language dictionaries. Tick the languages you actually dictate in.")
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                Repeater {
                    model: parent.parent.parent.languageOptions
                    delegate: CheckBox {
                        required property var modelData
                        text: modelData.name + "  (" + modelData.code + ")"
                        checked: appSettings.enabledLanguages.indexOf(modelData.code) >= 0
                        onToggled: {
                            const list = appSettings.enabledLanguages.slice()
                            const idx = list.indexOf(modelData.code)
                            if (checked && idx < 0) list.push(modelData.code)
                            if (!checked && idx >= 0) list.splice(idx, 1)
                            appSettings.enabledLanguages = list
                        }
                    }
                }
            }
        }
    }
}
