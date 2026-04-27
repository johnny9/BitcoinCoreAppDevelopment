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
    objectName: "settingsWallet"

    signal back

    background: null

    header: NavigationBar2 {
        leftItem: NavButton {
            objectName: "settingsWalletBack"
            iconSource: "image://images/caret-left"
            text: qsTr("Back")
            onClicked: root.back()
        }
        centerItem: Header {
            headerBold: true
            headerSize: 18
            header: qsTr("External Signer")
        }
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: width
        clip: true

        ColumnLayout {
            width: Math.min(parent.width, 450)
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 0

            WalletSettings {
                Layout.fillWidth: true
            }
        }
    }
}
