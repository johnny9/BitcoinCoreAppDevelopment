// Copyright (c) 2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.bitcoincore.qt 1.0

Button {
    property int bgRadius: 0
    property color bgDefaultColor: "transparent"
    property color bgHoverColor: Theme.color.neutral2
    property color textColor: Theme.color.neutral7
    property color textHoverColor: Theme.color.neutral9
    property color textActiveColor: Theme.color.neutral7

    id: root
    checkable: true
    checked: optionSwitch.checked
    hoverEnabled: AppMode.isDesktop
    padding: 0

    implicitWidth: 280
    implicitHeight: 33

    MouseArea {
        anchors.fill: parent
        enabled: false
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
    }

    onClicked: {
        optionSwitch.checked = !optionSwitch.checked
    }

    contentItem: RowLayout {
        spacing: 5
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 5

        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            spacing: 7

            CoreText {
                id: buttonText
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                horizontalAlignment: Text.AlignLeft
                font.pixelSize: 15
                text: root.text
            }
        }

        Item {
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: 38
            Layout.preferredHeight: 22

            OptionSwitch {
                id: optionSwitch
                anchors.centerIn: parent
                width: 38
                height: 22
                checked: root.checked
            }
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
            PropertyChanges { target: bg; color: root.bgHoverColor }
            PropertyChanges { target: buttonText; color: root.textHoverColor }
        }
    ]
}
