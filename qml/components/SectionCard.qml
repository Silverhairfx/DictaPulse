// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DictaPulse

// Raised clay card: the building block of every settings page. Title gets the
// serif display face; content flows in a column underneath.
Item {
    id: card
    property string title: ""
    property string subtitle: ""
    default property alias content: contentColumn.data
    Layout.fillWidth: true
    implicitHeight: header.implicitHeight + contentColumn.implicitHeight + Theme.pad * 3

    ClaySurface {
        anchors.fill: parent
        tier: "raised"
        radius: Theme.radius
        color: Theme.bgRaised
        borderColor: Theme.border
        borderWidth: 1
    }

    ColumnLayout {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: Theme.pad
        spacing: 3
        Label {
            text: card.title
            color: Theme.text
            font.family: Theme.displayFont
            font.pixelSize: 17
            visible: text !== ""
        }
        Label {
            text: card.subtitle
            color: Theme.textDim
            font.pixelSize: 12
            visible: text !== ""
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }

    ColumnLayout {
        id: contentColumn
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.topMargin: Theme.gap
        anchors.leftMargin: Theme.pad
        anchors.rightMargin: Theme.pad
        anchors.bottomMargin: Theme.pad
        spacing: Theme.gap
    }
}
