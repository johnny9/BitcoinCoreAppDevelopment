// Copyright (c) 2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../components"
import "../controls"

OptionPopup {
    id: root

    implicitWidth: 300
    implicitHeight: 50

    CoreText {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        font.pixelSize: 18
        text: qsTr("Enable coin control")
    }
    OptionSwitch {
        id: optionSwitch
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        checked: true
        width: 45
        height: 28
    }
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            optionSwitch.checked = !optionSwitch.checked
        }
    }
}