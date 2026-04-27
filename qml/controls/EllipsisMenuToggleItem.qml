// Copyright (c) 2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.bitcoincore.qt 1.0

Button {
    property int bgRadius: 5
    property color bgDefaultColor: "transparent"

    id: root
    checkable: true
    checked: optionSwitch.checked
    hoverEnabled: AppMode.isDesktop

    implicitWidth: 280
    implicitHeight: 44

    HoverHandler { cursorShape: Qt.PointingHandCursor }

    onClicked: {
        optionSwitch.checked = !optionSwitch.checked
    }

    contentItem: RowLayout {
        spacing: 7
        anchors.fill: parent
        anchors.margins: 10
        CoreText {
            id: buttonText
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            font.pixelSize: 15
            text: root.text
        }
        OptionSwitch {
            id: optionSwitch
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: 40
            Layout.preferredHeight: 24
            checked: root.checked
        }
    }

    background: Rectangle {
        id: bg
        color: root.bgDefaultColor
        radius: root.bgRadius

        Behavior on color {
            ColorAnimation { duration: 150 }
        }
    }

    states: [
        State {
            name: "HOVER"; when: root.hovered
            PropertyChanges { target: buttonText; color: Theme.color.orange }
        }
    ]
}
