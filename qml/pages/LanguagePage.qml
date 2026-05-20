import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DictaPulse

ScrollView {
    id: pageRoot
    clip: true

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

    ColumnLayout {
        width: pageRoot.width
        spacing: Theme.gap

        SectionCard {
            title: qsTr("Default language")
            subtitle: qsTr("Speech is transcribed in this language unless auto-detect is on. English-only models (model name ends in .en) always use English regardless of this setting.")
            SettingRow {
                label: qsTr("Default")
                ComboBox {
                    width: 220
                    textRole: "name"
                    valueRole: "code"
                    model: pageRoot.languageOptions
                    Component.onCompleted: {
                        const idx = indexOfValue(appSettings.defaultLanguage)
                        if (idx >= 0) currentIndex = idx
                    }
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

            Repeater {
                model: pageRoot.languageOptions
                delegate: CheckBox {
                    id: langCheck
                    required property var modelData
                    Layout.fillWidth: true
                    text: modelData.name + "  (" + modelData.code + ")"
                    checked: appSettings.enabledLanguages.indexOf(modelData.code) >= 0
                    onToggled: {
                        const list = appSettings.enabledLanguages.slice()
                        const idx = list.indexOf(modelData.code)
                        if (langCheck.checked && idx < 0) list.push(modelData.code)
                        if (!langCheck.checked && idx >= 0) list.splice(idx, 1)
                        appSettings.enabledLanguages = list
                    }
                }
            }
        }
    }
}
