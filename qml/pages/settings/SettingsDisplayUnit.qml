// Copyright (c) 2026 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../../controls"
import "../../components"

Page {
    id: root
    signal back

    objectName: "settingsDisplayUnitPage"
    background: null
    implicitWidth: 450
    leftPadding: 20
    rightPadding: 20
    topPadding: 30

    header: NavigationBar2 {
        leftItem: NavButton {
            objectName: "settingsDisplayUnitBack"
            iconSource: "image://images/caret-left"
            text: qsTr("Back")
            onClicked: root.back()
        }
        centerItem: Header {
            headerBold: true
            headerSize: 18
            header: qsTr("Display unit")
        }
    }

    ColumnLayout {
        spacing: 15
        width: Math.min(parent.width, 450)
        anchors.horizontalCenter: parent.horizontalCenter

        ButtonGroup {
            id: unitGroup
        }

        OptionButton {
            objectName: "displayUnitBTC"
            Layout.fillWidth: true
            ButtonGroup.group: unitGroup
            text: qsTr("BTC")
            description: qsTr("8 decimal places (0.00000001 BTC = 1 sat)")
            checked: optionsModel.displayUnit === 0
            onClicked: optionsModel.displayUnit = 0
        }

        OptionButton {
            objectName: "displayUnitSAT"
            Layout.fillWidth: true
            ButtonGroup.group: unitGroup
            text: qsTr("sat")
            description: qsTr("Satoshi, the smallest unit (1 sat = 0.00000001 BTC)")
            checked: optionsModel.displayUnit === 1
            onClicked: optionsModel.displayUnit = 1
        }
    }
}
