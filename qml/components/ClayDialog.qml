// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
import QtQuick
import QtQuick.Controls
// Qualified alias: the module's own Overlay.qml (the floating pill window)
// shadows QtQuick.Controls' Overlay attached type inside this module.
import QtQuick.Controls as Controls
import DictaPulse

// Clay modal: a floating clay card with a serif headline and clay buttons.
Dialog {
    id: control
    modal: true
    padding: Theme.pad
    topPadding: Theme.pad

    enter: Transition {
        NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 130 }
        NumberAnimation { property: "scale"; from: 0.94; to: 1; duration: 160; easing.type: Easing.OutCubic }
    }
    exit: Transition {
        NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 100 }
    }

    background: ClaySurface {
        tier: "raised"
        radius: Theme.radius + 4
        color: Theme.bgRaised
        borderColor: Theme.border
        borderWidth: 1
    }

    header: Label {
        text: control.title
        visible: control.title !== ""
        color: Theme.text
        font.family: Theme.displayFont
        font.pixelSize: 19
        leftPadding: Theme.pad
        rightPadding: Theme.pad
        topPadding: Theme.pad
    }

    footer: DialogButtonBox {
        visible: count > 0
        spacing: 8
        padding: Theme.pad
        topPadding: 0
        alignment: Qt.AlignRight
        background: null
        delegate: ClayButton {
            variant: DialogButtonBox.buttonRole === DialogButtonBox.AcceptRole
                     || DialogButtonBox.buttonRole === DialogButtonBox.YesRole
                     ? "default" : "outline"
        }
    }

    Controls.Overlay.modal: Rectangle {
        color: Theme.mode === "dark" ? Qt.rgba(0, 0, 0, 0.6) : Qt.rgba(0.13, 0.11, 0.21, 0.35)
        Behavior on opacity { NumberAnimation { duration: 120 } }
    }
}
