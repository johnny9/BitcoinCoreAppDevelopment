// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.bitcoincore.qt 1.0

import "../../controls"
import "../../components"

Popup {
    id: popup
    modal: true
    padding: 0

    x: 0
    y: 0
    width: parent.width
    height: parent.height

    contentItem: Page {
        header: NavigationBar2 {
            centerItem: Header {
                headerBold: true
                headerSize: 18
                header: qsTr("Coin Selection")
            }
            rightItem: NavButton {
                text: qsTr("Done")
                onClicked: popup.close()
            }
        }

        background: Rectangle {
            color: Theme.color.neutral0
        }
    }
}