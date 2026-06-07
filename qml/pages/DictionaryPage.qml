import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DictaPulse

ScrollView {
    id: pageRoot
    clip: true

    // The dictionary is stored as a JSON array in settings. Mirror it into a
    // local JS array the Repeater renders; write back on every committed edit.
    property var entries: []

    property var langOptions: [
        { code: "",   name: qsTr("All languages") },
        { code: "en", name: qsTr("English") },
        { code: "ar", name: qsTr("Arabic") },
        { code: "it", name: qsTr("Italian") },
        { code: "fr", name: qsTr("French") },
        { code: "de", name: qsTr("German") },
        { code: "es", name: qsTr("Spanish") }
    ]

    function load() {
        var parsed = []
        try { parsed = JSON.parse(appSettings.dictionary || "[]") } catch (e) { parsed = [] }
        entries = Array.isArray(parsed) ? parsed : []
    }
    function save() { appSettings.dictionary = JSON.stringify(entries) }
    function addEntry() {
        entries = entries.concat([{ from: "", to: "", caseSensitive: false, wholeWord: true, lang: "" }])
        save()
    }
    function updateEntry(i, key, val) {
        var e = entries.slice()
        e[i] = Object.assign({}, e[i]); e[i][key] = val
        entries = e; save()
    }
    function removeEntry(i) {
        var e = entries.slice(); e.splice(i, 1)
        entries = e; save()
    }
    function langName(code) {
        for (var i = 0; i < langOptions.length; ++i)
            if (langOptions[i].code === code) return langOptions[i].name
        return code
    }
    Component.onCompleted: load()

    ColumnLayout {
        width: pageRoot.width
        spacing: Theme.gap

        SectionCard {
            title: qsTr("Personal dictionary")
            subtitle: qsTr("Automatic find-and-replace applied to every transcript before it's inserted. Use it for names, jargon, and brand spellings Whisper gets wrong — e.g. \"git hub\" → \"GitHub\". The replacement's exact casing is preserved.")

            SettingRow {
                label: qsTr("Bias Whisper toward these spellings")
                hint: qsTr("Feed your replacement targets to Whisper as a hint so it's more likely to transcribe them correctly in the first place.")
                ClaySwitch {
                    checked: appSettings.dictionaryBias
                    onToggled: appSettings.dictionaryBias = checked
                }
            }
        }

        SectionCard {
            title: qsTr("Entries")
            subtitle: pageRoot.entries.length === 0
                      ? qsTr("No entries yet. Add your first replacement below.")
                      : qsTr("%1 replacement(s). Matching is case-insensitive unless you tick Aa.").arg(pageRoot.entries.length)

            // Column headers
            RowLayout {
                Layout.fillWidth: true
                visible: pageRoot.entries.length > 0
                spacing: Theme.padSm
                Label { text: qsTr("Replace"); color: Theme.textDim; font.pixelSize: 11; Layout.preferredWidth: 200 }
                Label { text: qsTr("With"); color: Theme.textDim; font.pixelSize: 11; Layout.preferredWidth: 200 }
                Label { text: qsTr("Lang"); color: Theme.textDim; font.pixelSize: 11; Layout.preferredWidth: 130 }
                Label { text: "Aa"; color: Theme.textDim; font.pixelSize: 11; ToolTip.text: qsTr("Case-sensitive") }
                Label { text: qsTr("Word"); color: Theme.textDim; font.pixelSize: 11 }
                Item { Layout.fillWidth: true }
            }

            Repeater {
                model: pageRoot.entries
                delegate: RowLayout {
                    id: row
                    required property int index
                    required property var modelData
                    Layout.fillWidth: true
                    spacing: Theme.padSm

                    ClayTextField {
                        Layout.preferredWidth: 200
                        text: row.modelData.from
                        placeholderText: qsTr("git hub")
                        onEditingFinished: pageRoot.updateEntry(row.index, "from", text)
                    }
                    ClayTextField {
                        Layout.preferredWidth: 200
                        text: row.modelData.to
                        placeholderText: "GitHub"
                        onEditingFinished: pageRoot.updateEntry(row.index, "to", text)
                    }
                    ClayComboBox {
                        Layout.preferredWidth: 130
                        textRole: "name"
                        valueRole: "code"
                        model: pageRoot.langOptions
                        Component.onCompleted: currentIndex = Math.max(0, indexOfValue(row.modelData.lang || ""))
                        onActivated: pageRoot.updateEntry(row.index, "lang", currentValue)
                    }
                    ClayCheckBox {
                        checked: row.modelData.caseSensitive === true
                        onToggled: pageRoot.updateEntry(row.index, "caseSensitive", checked)
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Case-sensitive match")
                    }
                    ClayCheckBox {
                        checked: row.modelData.wholeWord !== false
                        onToggled: pageRoot.updateEntry(row.index, "wholeWord", checked)
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Whole-word match only")
                    }
                    Item { Layout.fillWidth: true }
                    ClayButton {
                        text: "✕"
                        variant: "ghost"
                        small: true
                        onClicked: pageRoot.removeEntry(row.index)
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                ClayButton {
                    text: qsTr("Add entry")
                    variant: "secondary"
                    onClicked: pageRoot.addEntry()
                }
                Item { Layout.fillWidth: true }
            }
        }
    }
}
