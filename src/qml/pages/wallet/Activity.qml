// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
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
            width: Math.min(parent.width - 40, 600)
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: title.bottom
            anchors.bottom: parent.bottom

            model: ListModel {
                ListElement { label: "Payment from Alex for freelance coding work."; direction: "receiving"; date: "July 11"; amount: "₿ +0.00 001 000"; confirmed: false }
                ListElement { label: "Coffee from xpub."; direction: "sending"; date: "July 12"; amount: "₿ -0.00 001 000"; confirmed: true  }
                ListElement { label: "Received rent payment from Lisa."; direction: "receiving"; date: "February 2"; amount: "₿ +0.00 001 000"; confirmed: true  }
                ListElement { label: "Groceries at Satoshi Mart."; direction: "sending"; date: "January 21"; amount: "₿ -0.00 001 000"; confirmed: true }
            }

            delegate: ItemDelegate {
                id: delegate
                required property string label;
                required property string direction;
                required property string date;
                required property string amount;
                required property bool confirmed;

                width: listView.width
                height: 51

                background: Item {
                    Separator {
                        anchors.bottom: parent.bottom
                        width: parent.width
                    }
                }

                contentItem: Item {
                    Icon {
                        id: directionIcon
                        anchors.left: parent.left
                        anchors.margins: 6
                        anchors.verticalCenter: parent.verticalCenter
                        source: delegate.direction == "receiving" ? "qrc:/icons/triangle-down" : "qrc:/icons/triangle-up"
                        color: {
                            if (delegate.confirmed) {
                                if (delegate.direction == "receiving") {
                                    Theme.color.green
                                } else {
                                    Theme.color.orange
                                }
                            } else {
                                Theme.color.blue
                            }
                        }
                        size: 12
                    }
                    CoreText {
                        anchors.left: directionIcon.right
                        anchors.right: date.left
                        anchors.margins: 6
                        anchors.verticalCenter: parent.verticalCenter
                        text: delegate.label
                        font.pixelSize: 15
                        horizontalAlignment: Text.AlignLeft
                    }
                    CoreText {
                        id: date
                        anchors.right: amount.left
                        anchors.margins: 6
                        anchors.verticalCenter: parent.verticalCenter
                        text: delegate.date
                        font.pixelSize: 15
                        horizontalAlignment: Text.AlignRight
                    }

                    CoreText {
                        id: amount
                        anchors.right: parent.right
                        anchors.margins: 6
                        anchors.verticalCenter: parent.verticalCenter
                        text: delegate.amount
                        font.pixelSize: 15
                        color: delegate.direction == "receiving" ? Theme.color.green : Theme.color.neutral9
                    }
                }
            }
        }
    }
}
