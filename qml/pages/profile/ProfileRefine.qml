// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DictaPulse

// Refine: real-time text shaping. Each toggle drives the offline rules where
// feasible AND is described to the cleanup LLM via ProfileContext, so it works
// whichever engine the user runs.
ScrollView {
    id: root
    clip: true

    ColumnLayout {
        width: root.width
        spacing: Theme.gap

        SectionCard {
            title: qsTr("Refine")
            subtitle: qsTr("Pulse goes beyond basic dictation - cleaning up speech as you go. These preferences shape every transcript and are shared with whichever AI model you've connected.")

            SettingRow {
                label: qsTr("Remove fillers")
                hint: qsTr("Drops “um”, “uh”, and other pauses so your text stays clean and natural.")
                ClaySwitch {
                    checked: appSettings.removeFillerWords
                    onToggled: appSettings.removeFillerWords = checked
                }
            }
            SettingRow {
                label: qsTr("Auto-punctuation")
                hint: qsTr("Detects punctuation from your pauses and phrasing. You can still dictate marks like “comma” or “question mark” for precision.")
                ClaySwitch {
                    checked: appSettings.refineAutoPunctuation
                    onToggled: appSettings.refineAutoPunctuation = checked
                }
            }
            SettingRow {
                label: qsTr("Smart lists")
                hint: qsTr("Speak the numbers - “for 1. Apples 2. Bananas 3. Oranges” - and Pulse turns it into a formatted list.")
                ClaySwitch {
                    checked: appSettings.refineSmartLists
                    onToggled: appSettings.refineSmartLists = checked
                }
            }
            SettingRow {
                label: qsTr("Backtrack corrections")
                hint: qsTr("Pulse understands when you change your mind. Say “Let’s meet at 2… actually 3,” and it keeps the corrected version. (Needs an LLM cleanup provider for full effect.)")
                ClaySwitch {
                    checked: appSettings.refineBacktrack
                    onToggled: appSettings.refineBacktrack = checked
                }
            }
        }

        SectionCard {
            title: qsTr("How it reaches the AI")
            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                color: Theme.textDim
                font.pixelSize: 12
                text: qsTr("Offline rules apply instantly. When you connect a local or remote LLM on the Cleanup page, these same preferences are added to its instructions so it formats lists, fixes punctuation, applies corrections, and removes fillers in your voice - not a generic one.")
            }
        }
    }
}
