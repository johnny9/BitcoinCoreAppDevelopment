// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15
import QtQuick.Controls 2.15
import org.bitcoincore.qt 1.0

import "../../controls"
import "../../components"

PageStack {
    id: stackView

    initialItem: Page {
        id: root
        background: null

        CoreText {
            id: title
            text: qsTr("Activity")
            anchors.left: listView.left
            anchors.top: parent.top
            anchors.topMargin: 20
            font.pixelSize: 21
            bold: true
        }

        ListView {
            id: listView
            clip: true
            width: Math.min(parent.width - 40, 450)
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: title.bottom
            anchors.bottom: parent.bottom

            model: ListModel {
                ListElement { label: "Payment from Alex for freelance coding work."}
                ListElement { label: "Coffee from xpub."}
                ListElement { label: "Received rent payment from Lisa."}
                ListElement { label: "Groceries at Satoshi Mart."}
            }

            delegate: ItemDelegate {
                id: delegate
                required property string label;

                width: listView.width

                background: Item {
                    Separator {
                        anchors.bottom: parent.bottom
                        width: parent.width
                    }
                }

                contentItem: CoreText {
                    text: delegate.label
                }

            }
        }
    }

}