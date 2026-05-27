import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
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

    // Key-presence is a method, not a property; mirror it into a property and
    // refresh on the secrets store's changed() signal.
    property bool keyPresent: false
    function refreshKey() { keyPresent = secrets.hasKey(appSettings.cleanupRemoteProvider) }
    Component.onCompleted: refreshKey()
    Connections { target: secrets; function onChanged(id) { pageRoot.refreshKey() } }

    ColumnLayout {
        width: pageRoot.width
        spacing: Theme.gap

        SectionCard {
            title: qsTr("Cleanup provider")
            subtitle: qsTr("How each transcript is polished before it's inserted. Rules is offline and instant; Local and Remote send the text to an LLM with your prompt (and, soon, your vocabulary).")
            SettingRow {
                label: qsTr("Provider")
                ComboBox {
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
                    ComboBox {
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
                        }
                    }
                    Button {
                        text: qsTr("Setup help")
                        flat: true
                        onClicked: localHelp.open()
                    }
                }
            }
            SettingRow {
                label: qsTr("Endpoint")
                hint: qsTr("Base URL including /v1")
                TextField {
                    Layout.preferredWidth: 280
                    text: appSettings.cleanupLocalEndpoint
                    onEditingFinished: appSettings.cleanupLocalEndpoint = text
                }
            }
            SettingRow {
                label: qsTr("Model")
                hint: qsTr("Leave blank to use whatever model is loaded (LM Studio)")
                TextField {
                    Layout.preferredWidth: 280
                    placeholderText: qsTr("e.g. llama-3.1-8b-instruct")
                    text: appSettings.cleanupLocalModel
                    onEditingFinished: appSettings.cleanupLocalModel = text
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
                    ComboBox {
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
                    Button {
                        text: qsTr("Get a key")
                        flat: true
                        onClicked: remoteHelp.open()
                    }
                }
            }
            SettingRow {
                label: qsTr("Model")
                TextField {
                    Layout.preferredWidth: 280
                    text: appSettings.cleanupRemoteModel
                    onEditingFinished: appSettings.cleanupRemoteModel = text
                }
            }
            SettingRow {
                visible: appSettings.cleanupRemoteProvider === "custom"
                label: qsTr("Endpoint")
                hint: qsTr("OpenAI-compatible base URL including /v1")
                TextField {
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
                    TextField {
                        id: keyField
                        Layout.preferredWidth: 240
                        echoMode: TextInput.Password
                        placeholderText: pageRoot.keyPresent ? "••••••••••••" : qsTr("Paste API key")
                    }
                    Button {
                        text: qsTr("Save")
                        enabled: keyField.text.length > 0
                        onClicked: {
                            secrets.setKey(appSettings.cleanupRemoteProvider, keyField.text)
                            keyField.text = ""
                        }
                    }
                    Button {
                        text: qsTr("Clear")
                        flat: true
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

            TextArea {
                Layout.fillWidth: true
                Layout.preferredHeight: 110
                wrapMode: TextArea.Wrap
                text: appSettings.cleanupSystemPrompt
                onEditingFinished: appSettings.cleanupSystemPrompt = text
                background: Rectangle {
                    color: Theme.bgHover
                    radius: Theme.radiusSm
                    border.color: Theme.border
                    border.width: 1
                }
            }
            SettingRow {
                label: qsTr("Use my vocabulary")
                hint: qsTr("Bias cleanup toward your saved terms (takes effect once the Vocabulary feature is added).")
                Switch {
                    checked: appSettings.cleanupUseVocab
                    onToggled: appSettings.cleanupUseVocab = checked
                }
            }
        }
    }

    Dialog {
        id: localHelp
        anchors.centerIn: Overlay.overlay
        modal: true
        title: qsTr("Local LLM setup")
        standardButtons: Dialog.Ok
        width: 460
        contentItem: Label {
            wrapMode: Text.WordWrap
            color: Theme.text
            text: qsTr("LM Studio:\n1. Install LM Studio and download a chat model (e.g. Llama 3.1 8B Instruct).\n2. Open the ‘Developer’ tab → Start Server. It listens on http://localhost:1234.\n3. Leave Model blank to use the loaded model.\n\nOllama:\n1. Install Ollama, then run:  ollama pull llama3.1\n2. Ollama serves an OpenAI-compatible API on http://localhost:11434/v1.\n3. Set Model to the pulled name, e.g. llama3.1.")
        }
    }

    Dialog {
        id: remoteHelp
        anchors.centerIn: Overlay.overlay
        modal: true
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
