// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DictaPulse

// Personalize: the usage dashboard + the things that make Pulse sound like you —
// your dictionary, voice templates, and tone. All feed the AI via ProfileContext.
ScrollView {
    id: root
    clip: true

    // --- dictionary model (JSON-backed) ---
    property var entries: []
    property var langOptions: [
        { code: "",   name: qsTr("All") },
        { code: "en", name: qsTr("English") },
        { code: "ar", name: qsTr("Arabic") },
        { code: "it", name: qsTr("Italian") },
        { code: "fr", name: qsTr("French") },
        { code: "de", name: qsTr("German") },
        { code: "es", name: qsTr("Spanish") }
    ]
    function loadDict() {
        var p = []; try { p = JSON.parse(appSettings.dictionary || "[]") } catch (e) { p = [] }
        entries = Array.isArray(p) ? p : []
    }
    function saveDict() { appSettings.dictionary = JSON.stringify(entries) }
    function addEntry() { entries = entries.concat([{ from: "", to: "", caseSensitive: false, wholeWord: true, lang: "" }]); saveDict() }
    function updateEntry(i, k, v) { var e = entries.slice(); e[i] = Object.assign({}, e[i]); e[i][k] = v; entries = e; saveDict() }
    function removeEntry(i) { var e = entries.slice(); e.splice(i, 1); entries = e; saveDict() }

    // --- voice templates model (JSON-backed) ---
    property var templates: []
    function loadTpl() {
        var p = []; try { p = JSON.parse(appSettings.voiceTemplates || "[]") } catch (e) { p = [] }
        templates = Array.isArray(p) ? p : []
    }
    function saveTpl() { appSettings.voiceTemplates = JSON.stringify(templates) }
    function addTpl() { templates = templates.concat([{ cue: "", expansion: "" }]); saveTpl() }
    function updateTpl(i, k, v) { var t = templates.slice(); t[i] = Object.assign({}, t[i]); t[i][k] = v; templates = t; saveTpl() }
    function removeTpl(i) { var t = templates.slice(); t.splice(i, 1); templates = t; saveTpl() }

    // --- usage stats ---
    property var daily: []
    property var apps: []
    function refreshStats() {
        daily = profileStats.dailySeries(14)
        apps = profileStats.topApps(5)
    }
    Connections { target: profileStats; function onChanged() { root.refreshStats() } }

    Component.onCompleted: { loadDict(); loadTpl(); refreshStats() }

    ColumnLayout {
        width: root.width
        spacing: Theme.gap

        // ---- Usage dashboard ----
        SectionCard {
            title: qsTr("Usage")
            subtitle: qsTr("Track adoption and impact — total words, top apps, and trends.")

            GridLayout {
                Layout.fillWidth: true
                columns: 3
                columnSpacing: Theme.gap
                rowSpacing: Theme.gap

                Repeater {
                    model: [
                        { label: qsTr("Total words"), value: profileStats.totalWords },
                        { label: qsTr("Sessions"),    value: profileStats.totalSessions },
                        { label: qsTr("Words today"), value: profileStats.wordsToday() }
                    ]
                    delegate: Item {
                        required property var modelData
                        Layout.fillWidth: true
                        implicitHeight: 84
                        ClaySurface {
                            anchors.fill: parent
                            tier: "pressed"
                            radius: Theme.radiusSm
                            color: Theme.bgWell
                            borderColor: Theme.border
                        }
                        ColumnLayout {
                            anchors.centerIn: parent
                            spacing: 2
                            CountUp {
                                value: modelData.value
                                font.pixelSize: 28
                                Layout.alignment: Qt.AlignHCenter
                            }
                            Label {
                                text: modelData.label
                                color: Theme.textDim
                                font.pixelSize: 11
                                Layout.alignment: Qt.AlignHCenter
                            }
                        }
                    }
                }
            }

            Label {
                text: qsTr("Words per day · last 14 days")
                color: Theme.textDim
                font.pixelSize: 11
                Layout.topMargin: 4
            }
            GlowChart {
                Layout.fillWidth: true
                implicitHeight: 150
                tint: Theme.accent
                bars: false
                values: root.daily.map(function (d) { return d.words })
                labels: root.daily.map(function (d) { return d.date })
            }

            Label {
                text: qsTr("Top apps")
                color: Theme.textDim
                font.pixelSize: 11
                visible: root.apps.length > 0
            }
            GlowChart {
                Layout.fillWidth: true
                implicitHeight: 140
                visible: root.apps.length > 0
                tint: Theme.info
                bars: true
                values: root.apps.map(function (a) { return a.words })
                labels: root.apps.map(function (a) { return a.app })
            }
            Label {
                visible: root.apps.length === 0
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                color: Theme.textDim
                font.pixelSize: 12
                text: qsTr("No dictation recorded yet — your trends and top apps will appear here as you use Pulse.")
            }
        }

        // ---- Tone & style ----
        SectionCard {
            title: qsTr("Tone & style")
            subtitle: qsTr("Let Pulse shape your tone so you always sound like you — formal in docs, casual in chats. Applied by your connected LLM cleanup provider.")
            SettingRow {
                label: qsTr("Tone")
                ClayComboBox {
                    Layout.preferredWidth: 240
                    textRole: "name"
                    valueRole: "value"
                    model: [
                        { value: "off",          name: qsTr("Off — don’t change my tone") },
                        { value: "neutral",      name: qsTr("Neutral & clear") },
                        { value: "formal",       name: qsTr("Formal") },
                        { value: "casual",       name: qsTr("Casual") },
                        { value: "enthusiastic", name: qsTr("Enthusiastic") },
                        { value: "technical",    name: qsTr("Technical") }
                    ]
                    Component.onCompleted: currentIndex = Math.max(0, indexOfValue(appSettings.toneStyle))
                    onActivated: appSettings.toneStyle = currentValue
                }
            }
        }

        // ---- Dictionary ----
        SectionCard {
            title: qsTr("Dictionary")
            subtitle: qsTr("Pulse learns your words. Add names, industry terms, or unique spellings — they’re applied to every transcript and used to bias Whisper. Replacement casing is preserved.")

            SettingRow {
                label: qsTr("Bias Whisper toward these spellings")
                ClaySwitch {
                    checked: appSettings.dictionaryBias
                    onToggled: appSettings.dictionaryBias = checked
                }
            }

            Repeater {
                model: root.entries
                delegate: RowLayout {
                    id: drow
                    required property int index
                    required property var modelData
                    Layout.fillWidth: true
                    spacing: Theme.padSm
                    ClayTextField {
                        Layout.preferredWidth: 180
                        text: drow.modelData.from
                        placeholderText: qsTr("heard as…")
                        onEditingFinished: root.updateEntry(drow.index, "from", text)
                    }
                    Label { text: "→"; color: Theme.textDim }
                    ClayTextField {
                        Layout.preferredWidth: 180
                        text: drow.modelData.to
                        placeholderText: qsTr("written as…")
                        onEditingFinished: root.updateEntry(drow.index, "to", text)
                    }
                    ClayComboBox {
                        Layout.preferredWidth: 100
                        textRole: "name"
                        valueRole: "code"
                        model: root.langOptions
                        Component.onCompleted: currentIndex = Math.max(0, indexOfValue(drow.modelData.lang || ""))
                        onActivated: root.updateEntry(drow.index, "lang", currentValue)
                    }
                    ClayCheckBox {
                        checked: drow.modelData.caseSensitive === true
                        onToggled: root.updateEntry(drow.index, "caseSensitive", checked)
                        ToolTip.visible: hovered; ToolTip.text: qsTr("Case-sensitive")
                    }
                    Item { Layout.fillWidth: true }
                    ClayButton { text: "✕"; variant: "ghost"; small: true; onClicked: root.removeEntry(drow.index) }
                }
            }
            RowLayout {
                Layout.fillWidth: true
                ClayButton { text: qsTr("Add word"); variant: "secondary"; onClicked: root.addEntry() }
                Item { Layout.fillWidth: true }
            }
        }

        // ---- Voice templates ----
        SectionCard {
            title: qsTr("Voice templates")
            subtitle: qsTr("Create shortcuts for things you say often — scheduling links, intros, FAQs. Speak the cue and Pulse pastes the full text exactly where you need it.")

            Repeater {
                model: root.templates
                delegate: ColumnLayout {
                    id: trow
                    required property int index
                    required property var modelData
                    Layout.fillWidth: true
                    spacing: 4
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Theme.padSm
                        ClayTextField {
                            Layout.preferredWidth: 220
                            text: trow.modelData.cue
                            placeholderText: qsTr("cue phrase (e.g. “my booking link”)")
                            onEditingFinished: root.updateTpl(trow.index, "cue", text)
                        }
                        Item { Layout.fillWidth: true }
                        ClayButton { text: "✕"; variant: "ghost"; small: true; onClicked: root.removeTpl(trow.index) }
                    }
                    ClayTextArea {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 64
                        text: trow.modelData.expansion
                        placeholderText: qsTr("full text to paste…")
                        onEditingFinished: root.updateTpl(trow.index, "expansion", text)
                    }
                    Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }
                }
            }
            RowLayout {
                Layout.fillWidth: true
                ClayButton { text: qsTr("Add template"); variant: "secondary"; onClicked: root.addTpl() }
                Item { Layout.fillWidth: true }
            }
        }
    }
}
