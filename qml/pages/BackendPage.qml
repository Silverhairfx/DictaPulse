// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DictaPulse

ScrollView {
    clip: true
    ColumnLayout {
        width: parent.width
        spacing: Theme.gap

        SectionCard {
            title: qsTr("Detected hardware")
            subtitle: qsTr("DictaPulse uses this to filter available backends and recommend defaults.")
            GridLayout {
                columns: 2
                Layout.fillWidth: true
                columnSpacing: Theme.pad
                rowSpacing: 6
                Label { text: qsTr("CPU"); color: Theme.textDim }
                Label { text: hardwareInfo.cpuModel; color: Theme.text; wrapMode: Text.Wrap; Layout.fillWidth: true }
                Label { text: qsTr("Threads"); color: Theme.textDim }
                Label { text: hardwareInfo.cpuThreads; color: Theme.text }
                Label { text: qsTr("RAM"); color: Theme.textDim }
                Label { text: hardwareInfo.ramHuman; color: Theme.text }
                Label { text: qsTr("GPU"); color: Theme.textDim }
                Label { text: hardwareInfo.gpuVendor + " · " + hardwareInfo.gpuModel; color: Theme.text }
                Label { text: qsTr("Acceleration"); color: Theme.textDim }
                RowLayout {
                    spacing: 6
                    StatusPill { text: "CPU"; tint: Theme.success }
                    StatusPill { visible: hardwareInfo.hasVulkan; text: "Vulkan"; tint: Theme.success }
                    StatusPill { visible: hardwareInfo.hasCuda;   text: "CUDA";   tint: Theme.success }
                    StatusPill { visible: hardwareInfo.hasRocm;   text: "ROCm";   tint: Theme.success }
                }
                Label { text: qsTr("Recommended"); color: Theme.textDim }
                Label { text: hardwareInfo.recommendedBackend; color: Theme.accent; font.weight: Font.DemiBold }
            }
        }

        SectionCard {
            title: qsTr("Compute mode")
            subtitle: qsTr("CPU is always available. GPU modes require DictaPulse to be built with the matching acceleration enabled (see Build options in the README).")

            SettingRow {
                label: qsTr("Mode")
                ClayComboBox {
                    width: 220
                    model: [
                        { value: "cpu",    label: qsTr("CPU only") },
                        { value: "gpu",    label: qsTr("GPU only") },
                        { value: "hybrid", label: qsTr("Hybrid (GPU + CPU fallback)") }
                    ]
                    textRole: "label"
                    valueRole: "value"
                    Component.onCompleted: currentIndex = indexOfValue(appSettings.backendMode)
                    onActivated: appSettings.backendMode = currentValue
                }
            }

            SettingRow {
                label: qsTr("Acceleration API")
                hint: qsTr("Used when compute mode is GPU or Hybrid.")
                ClayComboBox {
                    id: apiBox
                    width: 220
                    model: hardwareInfo.availableBackends()
                    Component.onCompleted: {
                        const idx = model.indexOf(appSettings.backendApi)
                        if (idx >= 0) currentIndex = idx
                    }
                    onActivated: appSettings.backendApi = currentText
                }
            }

            SettingRow {
                label: qsTr("CPU threads")
                hint: qsTr("More threads can speed up CPU transcription but use more power. Default is half of your logical cores.")
                ClaySpinBox {
                    from: 1
                    to: hardwareInfo.cpuThreads
                    value: appSettings.cpuThreads
                    onValueModified: appSettings.cpuThreads = value
                }
            }
        }
    }
}
