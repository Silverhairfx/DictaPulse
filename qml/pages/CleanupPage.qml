// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
import QtQuick
import QtQuick.Controls
import QtQuick.Controls as Controls
import QtQuick.Layouts
import DictaPulse

ScrollView {
    id: pageRoot
    clip: true

    readonly property string provider: appSettings.cleanupProvider

    property var providerOpts: [
        { value: "none",   name: qsTr("None — insert raw transcript") },
        { value: "rules",  name: qsTr("Rules — offline, instant") },
        { value: "local",  name: qsTr("Local LLM — Ollama / LM Studio") },
        { value: "remote", name: qsTr("Remote API — Anthropic / OpenAI") }
    ]
    property var localPresets: [
        { value: "lmstudio", name: "LM Studio",  endpoint: "http://localhost:1234/v1" },
        { value: "ollama",   name: "Ollama",     endpoint: "http://localhost:11434/v1" },
        { value: "custom",   name: qsTr("Custom"), endpoint: "" }
    ]
    property var remoteProviders: [
        { value: "anthropic", name: "Anthropic (Claude)", model: "claude-haiku-4-5" },
        { value: "openai",    name: "OpenAI (GPT)",       model: "gpt-4o-mini" },
        { value: "custom",    name: qsTr("Custom (OpenAI-compatible)"), model: "" }
    ]

    // Key-presence is a method; mirror it into a property, refresh on secrets changed().
    property bool keyPresent: false
    function refreshKey() { keyPresent = secrets.hasKey(appSettings.cleanupRemoteProvider) }
    Component.onCompleted: { refreshKey(); refreshLocalModels() }
    Connections { target: secrets; function onChanged(id) { pageRoot.refreshKey() } }

    // Downloaded models pulled live from the local server, so a headless/daemon
    // user picks one here without opening the LM Studio / Ollama GUI.
    property var localModelOpts: []
    property bool localModelsFetched: false
    property bool localModelsBusy: false

    // Real models plus a trailing "Download a model" sentinel (the only entry when none exist).
    readonly property var localComboModel: {
        var base = localModelOpts.slice()
        base.push({
            name: (localModelsFetched && localModelOpts.length === 0)
                  ? qsTr("No models found — Download a model…")
                  : qsTr("Download a model…"),
            value: "__download__"
        })
        return base
    }

    function refreshLocalModels() {
        var ep = (appSettings.cleanupLocalEndpoint || "").trim()
        localModelsFetched = false
        localModelsBusy = true
        if (ep === "") { localModelOpts = []; localModelsFetched = true; localModelsBusy = false; return }
        var url = ep.replace(/\/+$/, "") + "/models"
        var xhr = new XMLHttpRequest()
        xhr.onreadystatechange = function() {
            if (xhr.readyState !== XMLHttpRequest.DONE) return
            var opts = []
            if (xhr.status === 200) {
                try {
                    var arr = (JSON.parse(xhr.responseText).data) || []
                    for (var i = 0; i < arr.length; i++) {
                        var id = arr[i].id || arr[i].name
                        if (!id) continue
                        var low = ("" + id).toLowerCase()
                        // Skip non-chat models (embeddings, the bundled whisper STT).
                        if (low.indexOf("embed") >= 0 || low.indexOf("whisper") >= 0) continue
                        opts.push({ name: id, value: id })
                    }
                } catch (e) {}
            }
            localModelOpts = opts
            localModelsFetched = true
            localModelsBusy = false
        }
        xhr.open("GET", url)
        xhr.send()
    }

    // Point the combo at the saved model. Default to the first real model when the
    // saved one is gone (or never set) so cleanup always has a valid target.
    function syncModelSelection() {
        var i = modelCombo.indexOfValue(appSettings.cleanupLocalModel)
        if (i >= 0) { modelCombo.currentIndex = i; return }
        if (localModelOpts.length > 0) {
            appSettings.cleanupLocalModel = localModelOpts[0].value
            modelCombo.currentIndex = 0
        } else {
            modelCombo.currentIndex = modelCombo.count - 1
        }
    }
    onLocalComboModelChanged: syncModelSelection()

    ColumnLayout {
        width: pageRoot.width
        spacing: Theme.gap

        SectionCard {
            title: qsTr("Cleanup provider")
            subtitle: qsTr("How each transcript is polished before it's inserted. Rules is offline and instant; Local and Remote send the text to an LLM with your prompt (and, soon, your vocabulary).")
            SettingRow {
                label: qsTr("Provider")
                ClayComboBox {
                    Layout.preferredWidth: 300
                    textRole: "name"
                    valueRole: "value"
                    model: pageRoot.providerOpts
                    Component.onCompleted: {
                        const i = indexOfValue(appSettings.cleanupProvider)
                        if (i >= 0) currentIndex = i
                    }
                    onActivated: appSettings.cleanupProvider = currentValue
                }
            }
        }

        // --- Rules ---
        SectionCard {
            visible: pageRoot.provider === "rules"
            title: qsTr("Rules engine")
            subtitle: qsTr("Offline punctuation and spacing cleanup, including an Arabic punctuation layer (؟ ، ؛). The fine-grained toggles — capitalize sentences, remove fillers, trailing space — live on the Output page.")
        }

        // --- Local LLM ---
        SectionCard {
            visible: pageRoot.provider === "local"
            title: qsTr("Local LLM endpoint")
            subtitle: qsTr("Point at any OpenAI-compatible server running on your machine. Private — nothing leaves your computer.")

            SettingRow {
                label: qsTr("App")
                RowLayout {
                    spacing: Theme.padSm
                    ClayComboBox {
                        Layout.preferredWidth: 200
                        textRole: "name"
                        valueRole: "value"
                        model: pageRoot.localPresets
                        Component.onCompleted: {
                            const i = indexOfValue(appSettings.cleanupLocalPreset)
                            if (i >= 0) currentIndex = i
                        }
                        onActivated: {
                            appSettings.cleanupLocalPreset = currentValue
                            const ep = pageRoot.localPresets[currentIndex].endpoint
                            if (ep !== "") appSettings.cleanupLocalEndpoint = ep
                            pageRoot.refreshLocalModels()
                        }
                    }
                    ClayButton {
                        text: qsTr("Setup help")
                        variant: "ghost"
                        onClicked: localHelp.open()
                    }
                }
            }
            SettingRow {
                label: qsTr("Endpoint")
                hint: qsTr("Base URL including /v1")
                ClayTextField {
                    Layout.preferredWidth: 280
                    text: appSettings.cleanupLocalEndpoint
                    onEditingFinished: {
                        appSettings.cleanupLocalEndpoint = text
                        pageRoot.refreshLocalModels()
                    }
                }
            }
            SettingRow {
                label: qsTr("Model")
                hint: qsTr("Picked automatically loads on use, no need to load it yourself")
                RowLayout {
                    spacing: Theme.padSm
                    ClayComboBox {
                        id: modelCombo
                        Layout.preferredWidth: 280
                        textRole: "name"
                        valueRole: "value"
                        model: pageRoot.localComboModel
                        onActivated: {
                            if (currentValue === "__download__") {
                                downloadHelp.open()
                                pageRoot.syncModelSelection()
                            } else {
                                appSettings.cleanupLocalModel = currentValue
                            }
                        }
                    }
                    ClayButton {
                        text: pageRoot.localModelsBusy ? qsTr("…") : qsTr("Refresh")
                        variant: "ghost"
                        enabled: !pageRoot.localModelsBusy
                        onClicked: pageRoot.refreshLocalModels()
                    }
                }
            }
        }

        // --- Remote API ---
        SectionCard {
            visible: pageRoot.provider === "remote"
            title: qsTr("Remote API")
            subtitle: qsTr("Best quality. Your transcript and prompt are sent to the provider. The API key is stored in your system keyring (KWallet), never in plain text.")

            SettingRow {
                label: qsTr("Provider")
                RowLayout {
                    spacing: Theme.padSm
                    ClayComboBox {
                        Layout.preferredWidth: 240
                        textRole: "name"
                        valueRole: "value"
                        model: pageRoot.remoteProviders
                        Component.onCompleted: {
                            const i = indexOfValue(appSettings.cleanupRemoteProvider)
                            if (i >= 0) currentIndex = i
                        }
                        onActivated: {
                            appSettings.cleanupRemoteProvider = currentValue
                            const m = pageRoot.remoteProviders[currentIndex].model
                            if (m !== "") appSettings.cleanupRemoteModel = m
                            pageRoot.refreshKey()
                        }
                    }
                    ClayButton {
                        text: qsTr("Get a key")
                        variant: "ghost"
                        onClicked: remoteHelp.open()
                    }
                }
            }
            SettingRow {
                label: qsTr("Model")
                ClayTextField {
                    Layout.preferredWidth: 280
                    text: appSettings.cleanupRemoteModel
                    onEditingFinished: appSettings.cleanupRemoteModel = text
                }
            }
            SettingRow {
                visible: appSettings.cleanupRemoteProvider === "custom"
                label: qsTr("Endpoint")
                hint: qsTr("OpenAI-compatible base URL including /v1")
                ClayTextField {
                    Layout.preferredWidth: 280
                    text: appSettings.cleanupRemoteEndpoint
                    onEditingFinished: appSettings.cleanupRemoteEndpoint = text
                }
            }
            SettingRow {
                label: qsTr("API key")
                hint: pageRoot.keyPresent ? qsTr("A key is saved for %1").arg(appSettings.cleanupRemoteProvider)
                                          : qsTr("No key saved for %1").arg(appSettings.cleanupRemoteProvider)
                RowLayout {
                    spacing: Theme.padSm
                    ClayTextField {
                        id: keyField
                        Layout.preferredWidth: 240
                        echoMode: TextInput.Password
                        placeholderText: pageRoot.keyPresent ? "••••••••••••" : qsTr("Paste API key")
                    }
                    ClayButton {
                        text: qsTr("Save")
                        enabled: keyField.text.length > 0
                        onClicked: {
                            secrets.setKey(appSettings.cleanupRemoteProvider, keyField.text)
                            keyField.text = ""
                        }
                    }
                    ClayButton {
                        text: qsTr("Clear")
                        variant: "ghost"
                        visible: pageRoot.keyPresent
                        onClicked: secrets.clearKey(appSettings.cleanupRemoteProvider)
                    }
                }
            }
        }

        // --- Prompt + vocabulary (LLM providers only) ---
        SectionCard {
            visible: pageRoot.provider === "local" || pageRoot.provider === "remote"
            title: qsTr("Instructions")
            subtitle: qsTr("The system prompt sent with every cleanup request.")

            ClayTextArea {
                Layout.fillWidth: true
                Layout.preferredHeight: 110
                text: appSettings.cleanupSystemPrompt
                onEditingFinished: appSettings.cleanupSystemPrompt = text
            }
            SettingRow {
                label: qsTr("Use my vocabulary")
                hint: qsTr("Bias cleanup toward your saved terms (takes effect once the Vocabulary feature is added).")
                ClaySwitch {
                    checked: appSettings.cleanupUseVocab
                    onToggled: appSettings.cleanupUseVocab = checked
                }
            }
        }
    }

    ClayDialog {
        id: localHelp
        anchors.centerIn: Controls.Overlay.overlay
        title: qsTr("Local LLM setup")
        standardButtons: Dialog.Ok
        width: 460
        contentItem: Label {
            wrapMode: Text.WordWrap
            color: Theme.text
            text: qsTr("LM Studio:\n1. Install LM Studio and download a chat model (e.g. Llama 3.1 8B Instruct).\n2. Open the ‘Developer’ tab → Start Server. It listens on http://localhost:1234.\n3. Pick the model from the Model dropdown above.\n\nOllama:\n1. Install Ollama, then run:  ollama pull llama3.1\n2. Ollama serves an OpenAI-compatible API on http://localhost:11434/v1.\n3. Pick the pulled model from the Model dropdown above.")
        }
    }

    ClayDialog {
        id: downloadHelp
        anchors.centerIn: Controls.Overlay.overlay
        title: qsTr("Add a model")
        standardButtons: Dialog.Ok
        width: 480
        contentItem: Label {
            wrapMode: Text.WordWrap
            color: Theme.text
            text: appSettings.cleanupLocalPreset === "ollama"
                ? qsTr("Ollama\n\nGUI: open Ollama and pull a model.\nTerminal / daemon:  ollama pull llama3.1\n\nThen press Refresh here and pick it from the list. DictaPulse loads the model for you on each cleanup, so there's nothing else to start.")
                : qsTr("LM Studio\n\nApp: open the Search tab, find a chat model (e.g. Llama 3.1 8B Instruct), and Download.\nDaemon / no GUI:  lms get llama-3.1-8b-instruct\n\nThen press Refresh here and pick it from the list. DictaPulse loads the model for you on each cleanup. Models are read from the running server, so a custom download folder works automatically.")
        }
    }

    ClayDialog {
        id: remoteHelp
        anchors.centerIn: Controls.Overlay.overlay
        title: qsTr("Getting an API key")
        standardButtons: Dialog.Ok
        width: 460
        contentItem: Label {
            wrapMode: Text.WordWrap
            color: Theme.text
            text: qsTr("Anthropic (Claude):\nconsole.anthropic.com → API Keys → Create Key. Fast/cheap model: claude-haiku-4-5.\n\nOpenAI (GPT):\nplatform.openai.com → API keys → Create new secret key. Fast/cheap model: gpt-4o-mini.\n\nPaste the key above and press Save — it's stored in your system keyring, never in DictaPulse's config file.")
        }
    }
}
