import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DictaPulse

ScrollView {
    id: pageRoot
    clip: true

    // Per-app rules: JSON array of {match, mode} mirrored into a local array.
    property var rules: []
    property var modeOptions: [
        { value: "insert",     label: qsTr("Direct insertion") },
        { value: "copy-paste", label: qsTr("Copy + paste") },
        { value: "clipboard",  label: qsTr("Clipboard only") }
    ]
    function loadRules() {
        var parsed = []
        try { parsed = JSON.parse(appSettings.perAppRules || "[]") } catch (e) { parsed = [] }
        rules = Array.isArray(parsed) ? parsed : []
    }
    function saveRules() { appSettings.perAppRules = JSON.stringify(rules) }
    function addRule() { rules = rules.concat([{ match: "", mode: "copy-paste" }]); saveRules() }
    function updateRule(i, key, val) {
        var r = rules.slice(); r[i] = Object.assign({}, r[i]); r[i][key] = val
        rules = r; saveRules()
    }
    function removeRule(i) { var r = rules.slice(); r.splice(i, 1); rules = r; saveRules() }
    Component.onCompleted: loadRules()

    ColumnLayout {
        width: pageRoot.width
        spacing: Theme.gap

        SectionCard {
            title: qsTr("Output mode")
            subtitle: qsTr("How transcribed text reaches your active app.")

            SettingRow {
                label: qsTr("Mode")
                ClayComboBox {
                    width: 260
                    model: [
                        { value: "insert",     label: qsTr("Direct insertion (default)") },
                        { value: "copy-paste", label: qsTr("Copy + paste (Ctrl+V)") },
                        { value: "clipboard",  label: qsTr("Clipboard only — never paste") }
                    ]
                    textRole: "label"
                    valueRole: "value"
                    Component.onCompleted: currentIndex = indexOfValue(appSettings.outputMode)
                    onActivated: appSettings.outputMode = currentValue
                }
            }

            SettingRow {
                label: qsTr("Fall back to clipboard if insertion fails")
                hint: qsTr("Recommended on Wayland — direct injection can fail in privileged windows.")
                ClaySwitch {
                    checked: appSettings.clipboardFallback
                    onToggled: appSettings.clipboardFallback = checked
                }
            }

            SettingRow {
                label: qsTr("Add trailing space")
                ClaySwitch {
                    checked: appSettings.addTrailingSpace
                    onToggled: appSettings.addTrailingSpace = checked
                }
            }
        }

        SectionCard {
            title: qsTr("Text cleanup")
            SettingRow {
                label: qsTr("Cleanup punctuation & spacing")
                ClaySwitch {
                    checked: appSettings.cleanupEnabled
                    onToggled: appSettings.cleanupEnabled = checked
                }
            }
            SettingRow {
                label: qsTr("Capitalize sentences")
                ClaySwitch {
                    checked: appSettings.capitalizeSentences
                    onToggled: appSettings.capitalizeSentences = checked
                }
            }
            SettingRow {
                label: qsTr("Remove filler words")
                hint: qsTr("Drops common English fillers: um, uh, erm, mm, hmm. English only.")
                ClaySwitch {
                    checked: appSettings.removeFillerWords
                    onToggled: appSettings.removeFillerWords = checked
                }
            }
        }

        SectionCard {
            title: qsTr("Per-app output rules")
            subtitle: qsTr("Override the output mode for specific apps. The first rule whose text appears in the focused window's app-id wins — e.g. \"konsole\" → Copy + paste so dictation lands in your terminal. Falls back to the default mode above when nothing matches.")

            SettingRow {
                label: qsTr("Enable per-app rules")
                hint: qsTr("On KDE Wayland this reads the focused window via a small KWin script; on X11 via xdotool.")
                ClaySwitch {
                    checked: appSettings.perAppRulesEnabled
                    onToggled: appSettings.perAppRulesEnabled = checked
                }
            }

            // Live "what's focused right now" helper to make authoring rules easy.
            RowLayout {
                Layout.fillWidth: true
                visible: appSettings.perAppRulesEnabled
                spacing: Theme.padSm
                Label {
                    text: qsTr("Focused app:")
                    color: Theme.textDim
                    font.pixelSize: 12
                }
                Label {
                    id: detectedApp
                    property string value: ""
                    text: value === "" ? qsTr("(press Detect)") : value
                    color: value === "" ? Theme.textDim : Theme.accent
                    font.pixelSize: 12
                    font.family: "monospace"
                }
                ClayButton {
                    text: qsTr("Detect")
                    variant: "ghost"
                    small: true
                    onClicked: detectedApp.value = controller.activeWindowId()
                }
                Item { Layout.fillWidth: true }
            }

            Repeater {
                model: pageRoot.rules
                delegate: RowLayout {
                    id: ruleRow
                    required property int index
                    required property var modelData
                    Layout.fillWidth: true
                    visible: appSettings.perAppRulesEnabled
                    spacing: Theme.padSm

                    ClayTextField {
                        Layout.preferredWidth: 240
                        text: ruleRow.modelData.match
                        placeholderText: qsTr("app-id contains… (e.g. konsole)")
                        onEditingFinished: pageRoot.updateRule(ruleRow.index, "match", text)
                    }
                    Label { text: "→"; color: Theme.textDim }
                    ClayComboBox {
                        Layout.preferredWidth: 200
                        textRole: "label"
                        valueRole: "value"
                        model: pageRoot.modeOptions
                        Component.onCompleted: currentIndex = Math.max(0, indexOfValue(ruleRow.modelData.mode))
                        onActivated: pageRoot.updateRule(ruleRow.index, "mode", currentValue)
                    }
                    Item { Layout.fillWidth: true }
                    ClayButton {
                        text: "✕"
                        variant: "ghost"
                        small: true
                        onClicked: pageRoot.removeRule(ruleRow.index)
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                visible: appSettings.perAppRulesEnabled
                ClayButton {
                    text: qsTr("Add rule")
                    variant: "secondary"
                    onClicked: pageRoot.addRule()
                }
                Item { Layout.fillWidth: true }
            }
        }
    }
}
