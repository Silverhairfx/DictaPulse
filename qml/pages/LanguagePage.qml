// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DictaPulse

ScrollView {
    id: pageRoot
    clip: true

    // The full set of languages Whisper multilingual models can transcribe.
    // Common dictation languages are listed first; everything else follows.
    // Use the search box below to find one quickly.
    property var languageOptions: [
        { code: "en", name: qsTr("English") },
        { code: "ar", name: qsTr("Arabic") },
        { code: "es", name: qsTr("Spanish") },
        { code: "fr", name: qsTr("French") },
        { code: "de", name: qsTr("German") },
        { code: "it", name: qsTr("Italian") },
        { code: "pt", name: qsTr("Portuguese") },
        { code: "ru", name: qsTr("Russian") },
        { code: "zh", name: qsTr("Chinese") },
        { code: "ja", name: qsTr("Japanese") },
        { code: "ko", name: qsTr("Korean") },
        { code: "hi", name: qsTr("Hindi") },
        { code: "tr", name: qsTr("Turkish") },
        { code: "nl", name: qsTr("Dutch") },
        { code: "pl", name: qsTr("Polish") },
        { code: "uk", name: qsTr("Ukrainian") },
        { code: "fa", name: qsTr("Persian") },
        { code: "ur", name: qsTr("Urdu") },
        { code: "id", name: qsTr("Indonesian") },
        { code: "vi", name: qsTr("Vietnamese") },
        { code: "he", name: qsTr("Hebrew") },
        { code: "sv", name: qsTr("Swedish") },
        { code: "el", name: qsTr("Greek") },
        { code: "cs", name: qsTr("Czech") },
        { code: "ro", name: qsTr("Romanian") },
        { code: "da", name: qsTr("Danish") },
        { code: "fi", name: qsTr("Finnish") },
        { code: "no", name: qsTr("Norwegian") },
        { code: "hu", name: qsTr("Hungarian") },
        { code: "th", name: qsTr("Thai") },
        { code: "ms", name: qsTr("Malay") },
        { code: "ca", name: qsTr("Catalan") },
        { code: "ta", name: qsTr("Tamil") },
        { code: "hr", name: qsTr("Croatian") },
        { code: "bg", name: qsTr("Bulgarian") },
        { code: "lt", name: qsTr("Lithuanian") },
        { code: "la", name: qsTr("Latin") },
        { code: "mi", name: qsTr("Maori") },
        { code: "ml", name: qsTr("Malayalam") },
        { code: "cy", name: qsTr("Welsh") },
        { code: "sk", name: qsTr("Slovak") },
        { code: "te", name: qsTr("Telugu") },
        { code: "lv", name: qsTr("Latvian") },
        { code: "bn", name: qsTr("Bengali") },
        { code: "sr", name: qsTr("Serbian") },
        { code: "az", name: qsTr("Azerbaijani") },
        { code: "sl", name: qsTr("Slovenian") },
        { code: "kn", name: qsTr("Kannada") },
        { code: "et", name: qsTr("Estonian") },
        { code: "mk", name: qsTr("Macedonian") },
        { code: "br", name: qsTr("Breton") },
        { code: "eu", name: qsTr("Basque") },
        { code: "is", name: qsTr("Icelandic") },
        { code: "hy", name: qsTr("Armenian") },
        { code: "ne", name: qsTr("Nepali") },
        { code: "mn", name: qsTr("Mongolian") },
        { code: "bs", name: qsTr("Bosnian") },
        { code: "kk", name: qsTr("Kazakh") },
        { code: "sq", name: qsTr("Albanian") },
        { code: "sw", name: qsTr("Swahili") },
        { code: "gl", name: qsTr("Galician") },
        { code: "mr", name: qsTr("Marathi") },
        { code: "pa", name: qsTr("Punjabi") },
        { code: "si", name: qsTr("Sinhala") },
        { code: "km", name: qsTr("Khmer") },
        { code: "sn", name: qsTr("Shona") },
        { code: "yo", name: qsTr("Yoruba") },
        { code: "so", name: qsTr("Somali") },
        { code: "af", name: qsTr("Afrikaans") },
        { code: "oc", name: qsTr("Occitan") },
        { code: "ka", name: qsTr("Georgian") },
        { code: "be", name: qsTr("Belarusian") },
        { code: "tg", name: qsTr("Tajik") },
        { code: "sd", name: qsTr("Sindhi") },
        { code: "gu", name: qsTr("Gujarati") },
        { code: "am", name: qsTr("Amharic") },
        { code: "yi", name: qsTr("Yiddish") },
        { code: "lo", name: qsTr("Lao") },
        { code: "uz", name: qsTr("Uzbek") },
        { code: "fo", name: qsTr("Faroese") },
        { code: "ht", name: qsTr("Haitian Creole") },
        { code: "ps", name: qsTr("Pashto") },
        { code: "tk", name: qsTr("Turkmen") },
        { code: "nn", name: qsTr("Nynorsk") },
        { code: "mt", name: qsTr("Maltese") },
        { code: "sa", name: qsTr("Sanskrit") },
        { code: "lb", name: qsTr("Luxembourgish") },
        { code: "my", name: qsTr("Myanmar") },
        { code: "bo", name: qsTr("Tibetan") },
        { code: "tl", name: qsTr("Tagalog") },
        { code: "mg", name: qsTr("Malagasy") },
        { code: "as", name: qsTr("Assamese") },
        { code: "tt", name: qsTr("Tatar") },
        { code: "haw", name: qsTr("Hawaiian") },
        { code: "ln", name: qsTr("Lingala") },
        { code: "ha", name: qsTr("Hausa") },
        { code: "ba", name: qsTr("Bashkir") },
        { code: "jw", name: qsTr("Javanese") },
        { code: "su", name: qsTr("Sundanese") },
        { code: "yue", name: qsTr("Cantonese") }
    ]

    property string filter: ""

    function filteredLanguages() {
        const f = filter.trim().toLowerCase()
        if (f === "")
            return languageOptions
        return languageOptions.filter(function(l) {
            return l.name.toLowerCase().indexOf(f) >= 0 || l.code.indexOf(f) >= 0
        })
    }

    ColumnLayout {
        width: pageRoot.width
        spacing: Theme.gap

        SectionCard {
            title: qsTr("Default language")
            subtitle: qsTr("Speech is transcribed in this language unless auto-detect is on. English-only models (model name ends in .en) always use English regardless of this setting.")
            SettingRow {
                label: qsTr("Default")
                ClayComboBox {
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
                hint: qsTr("Whisper detects the language from the first ~30s. Detection is restricted to your enabled languages below, so it never drifts to a wrong-but-similar one.")
                ClaySwitch {
                    checked: appSettings.autoDetectLanguage
                    onToggled: appSettings.autoDetectLanguage = checked
                }
            }
        }

        SectionCard {
            title: qsTr("Enabled languages")
            subtitle: qsTr("Tick every language you dictate in — Whisper supports them all. Auto-detect is constrained to this set; enabling exactly one forces that language. Also used for per-language vocabulary.")

            ClayTextField {
                Layout.fillWidth: true
                placeholderText: qsTr("Search languages…")
                text: pageRoot.filter
                onTextEdited: pageRoot.filter = text
            }

            Label {
                Layout.fillWidth: true
                visible: appSettings.enabledLanguages.length > 0
                text: qsTr("Enabled: %1").arg(appSettings.enabledLanguages.join(", "))
                color: Theme.textDim
                font.pixelSize: 11
                wrapMode: Text.WordWrap
            }

            Repeater {
                model: pageRoot.filteredLanguages()
                delegate: ClayCheckBox {
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
