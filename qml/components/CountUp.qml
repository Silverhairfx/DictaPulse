// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
import QtQuick
import DictaPulse

// Animated number that counts up from 0 to `value` - for dashboard stat tiles.
Text {
    id: root
    property int value: 0
    property int duration: 1100
    property real current: 0

    text: Math.round(current).toLocaleString(Qt.locale(), 'f', 0)
    color: Theme.text
    font.family: Theme.displayFont
    font.pixelSize: 30

    NumberAnimation {
        id: anim
        target: root
        property: "current"
        duration: root.duration
        easing.type: Easing.OutCubic
    }
    function play() { anim.from = 0; anim.to = root.value; anim.restart() }
    Component.onCompleted: play()
    onValueChanged: play()
}
