// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DictaPulse

RowLayout {
    id: row
    property string label: ""
    property string hint: ""
    default property alias rowContent: container.data
    Layout.fillWidth: true
    spacing: Theme.gap

    ColumnLayout {
        spacing: 2
        Layout.fillWidth: true
        Label {
            text: row.label
            color: Theme.text
            font.pixelSize: 13
        }
        Label {
            text: row.hint
            color: Theme.textDim
            font.pixelSize: 11
            visible: text !== ""
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }
    Item {
        id: container
        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
        implicitWidth: childrenRect.width
        implicitHeight: childrenRect.height
    }
}
