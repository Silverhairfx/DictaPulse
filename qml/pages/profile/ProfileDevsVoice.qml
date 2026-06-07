// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DictaPulse

// Devs Voice: dictation that understands code. Dev mode + a jargon vocabulary
// feed Whisper and the cleanup LLM with syntax-aware instructions.
ScrollView {
    id: root
    clip: true

    readonly property string commonTerms:
        "Supabase, Cloudflare, Vercel, Kubernetes, PostgreSQL, nginx, Redis, " +
        "GraphQL, TypeScript, Tailwind, Next.js, Prisma, Docker, Terraform, " +
        "camelCase, snake_case, async, await, npm, pnpm, kubectl, stdout, stderr"

    ColumnLayout {
        width: root.width
        spacing: Theme.gap

        SectionCard {
            title: qsTr("Developer mode")
            subtitle: qsTr("Built for developers, not just dictation. Pulse parses your speech with syntax awareness so code stays code — preserving camelCase, snake_case, CLI commands, and exact spacing.")

            SettingRow {
                label: qsTr("Enable developer mode")
                hint: qsTr("Adds syntax-aware instructions for your connected AI and biases Whisper toward your technical terms.")
                ClaySwitch {
                    checked: appSettings.devModeEnabled
                    onToggled: appSettings.devModeEnabled = checked
                }
            }
        }

        SectionCard {
            title: qsTr("Dev jargon")
            subtitle: qsTr("Terms Pulse should always recognize and spell correctly — frameworks, tools, project names. Comma- or line-separated. These are fed to Whisper and your LLM.")
            enabled: appSettings.devModeEnabled
            opacity: appSettings.devModeEnabled ? 1.0 : 0.5

            ClayTextArea {
                Layout.fillWidth: true
                Layout.preferredHeight: 120
                text: appSettings.devJargon
                placeholderText: qsTr("Supabase, Cloudflare, Vercel, myProjectName, useAuthStore…")
                onEditingFinished: appSettings.devJargon = text
            }
            RowLayout {
                Layout.fillWidth: true
                ClayButton {
                    text: qsTr("Load common terms")
                    variant: "secondary"
                    onClicked: appSettings.devJargon =
                        (appSettings.devJargon.trim().length > 0
                         ? appSettings.devJargon.trim() + ", " : "") + root.commonTerms
                }
                Item { Layout.fillWidth: true }
            }
        }

        SectionCard {
            title: qsTr("What dev mode does")
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8
                Repeater {
                    model: [
                        { t: qsTr("Syntax awareness"), d: qsTr("Keeps code as code — variables, CLI commands, exact spacing and formatting; no stray capitalization or punctuation in code tokens.") },
                        { t: qsTr("Dev jargon"), d: qsTr("Recognizes terms like Supabase, Cloudflare, and Vercel. Anything it misses, add above with a single correction.") },
                        { t: qsTr("File recognition (coming soon)"), d: qsTr("In Cursor / VS Code, Pulse will recognize filenames as you speak and tag the right file in your workspace for more precise AI responses.") }
                    ]
                    delegate: RowLayout {
                        required property var modelData
                        Layout.fillWidth: true
                        spacing: 10
                        Label { text: "›"; color: Theme.accent; font.pixelSize: 15; Layout.alignment: Qt.AlignTop }
                        ColumnLayout {
                            spacing: 1
                            Layout.fillWidth: true
                            Label { text: modelData.t; color: Theme.text; font.pixelSize: 13; font.weight: Font.DemiBold }
                            Label { text: modelData.d; color: Theme.textDim; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                        }
                    }
                }
            }
        }
    }
}
