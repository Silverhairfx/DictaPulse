// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
import QtQuick
import QtQuick.Controls
import DictaPulse

// Clay dropdown: a raised chip that sinks on press; the popup is a floating
// clay card with indigo-washed hover rows.
ComboBox {
    id: control
    hoverEnabled: true
    implicitHeight: 36
    opacity: enabled ? 1.0 : 0.5

    background: ClaySurface {
        tier: control.pressed || control.popup.visible ? "pressed" : "sm"
        radius: Theme.radiusSm
        color: Theme.bgRaised
        borderColor: control.activeFocus || control.popup.visible
                     ? Theme.accent
                     : control.hovered ? Theme.borderHi : Theme.border
        borderWidth: 1
        Behavior on borderColor { ColorAnimation { duration: 120 } }
    }

    contentItem: Label {
        leftPadding: 12
        rightPadding: 30
        text: control.displayText
        color: Theme.text
        font.pixelSize: 13
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    indicator: Label {
        x: control.width - width - 10
        anchors.verticalCenter: parent.verticalCenter
        text: "⌄"
        color: Theme.textDim
        font.pixelSize: 14
        font.weight: Font.Bold
        rotation: control.popup.visible ? 180 : 0
        Behavior on rotation { NumberAnimation { duration: 140 } }
    }

    delegate: ItemDelegate {
        id: row
        required property var model
        required property int index
        width: ListView.view ? ListView.view.width : control.width
        height: 32
        hoverEnabled: true

        background: Rectangle {
            anchors.fill: parent
            anchors.leftMargin: 4
            anchors.rightMargin: 4
            radius: 6
            color: row.hovered ? Theme.bgHover
                 : control.currentIndex === row.index ? Theme.accentSoft
                 : "transparent"
        }
        contentItem: Label {
            leftPadding: 8
            text: control.textRole !== ""
                  ? (row.model[control.textRole] !== undefined ? row.model[control.textRole]
                                                               : row.model.modelData[control.textRole])
                  : row.model.modelData ?? row.model.display
            color: control.currentIndex === row.index ? Theme.accentSoftFg : Theme.text
            font.pixelSize: 13
            font.weight: control.currentIndex === row.index ? Font.DemiBold : Font.Normal
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }

    popup: Popup {
        y: control.height + 6
        width: control.width
        padding: 5
        implicitHeight: Math.min(contentItem.implicitHeight + 10, 320)

        enter: Transition {
            NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 110 }
            NumberAnimation { property: "scale"; from: 0.96; to: 1; duration: 130; easing.type: Easing.OutCubic }
        }
        exit: Transition {
            NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 90 }
        }

        background: ClaySurface {
            tier: "raised"
            radius: Theme.radius
            color: Theme.bgRaised
            borderColor: Theme.border
            borderWidth: 1
        }

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
            ScrollBar.vertical: ScrollBar {}
        }
    }
}
