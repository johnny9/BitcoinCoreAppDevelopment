// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.bitcoincore.qt 1.0

import "../../controls"
import "../../components"
import "../settings"

Page {
    property PaymentRequest request

    id: root
    background: null

    header: NavigationBar2 {
        id: navbar
        leftItem: NavButton {
            iconSource: "image://images/caret-left"
            text: qsTr("Back")
            onClicked: {
                root.StackView.view.pop()
            }
        }
        centerItem: Item {
            id: header
            Layout.fillWidth: true

            CoreText {
                anchors.left: parent.left
                text: qsTr("Payment request")
                font.pixelSize: 21
                bold: true
            }
        }
        rightItem: Icon {
            anchors.right: parent.right
            source: "image://images/ellipsis"
            color: Theme.color.neutral9
            size: 40
        }
    }

    ScrollView {
        clip: true
        width: parent.width
        height: parent.height
        contentWidth: width

        ColumnLayout {
            id: columnLayout
            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(parent.width, 450)
            spacing: 30

            Icon {
                Layout.alignment: Qt.AlignHCenter
                source: "image://images/pending"
                width: 60
                height: width
            }

            CoreText {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Created just now")
                color: Theme.color.neutral7
                font.pixelSize: 18
            }

            Item {
                height: 50
                Layout.fillWidth: true
                CoreText {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    color: Theme.color.neutral7
                    text: qsTr("Label")
                    font.pixelSize: 15
                }

                TextField {
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    leftPadding: 0
                    font.family: "Inter"
                    font.styleName: "Regular"
                    font.pixelSize: 18
                    color: Theme.color.neutral9
                    placeholderTextColor: Theme.color.neutral7
                    background: Item {}
                    text: root.request.amount
                }
            }

            Item {
                BitcoinAmount {
                    id: bitcoinAmount
                }

                height: 50
                Layout.fillWidth: true
                CoreText {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    color: Theme.color.neutral7
                    text: qsTr("Amount")
                    font.pixelSize: 15
                }

                TextField {
                    id: bitcoinAmountText
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    leftPadding: 0
                    font.family: "Inter"
                    font.styleName: "Regular"
                    font.pixelSize: 18
                    color: Theme.color.neutral9
                    placeholderTextColor: Theme.color.neutral7
                    background: Item {}
                    placeholderText: "0.00000000"
                    text: root.request.amount
                    onTextChanged: {
                        bitcoinAmountText.text = bitcoinAmount.sanitize(bitcoinAmountText.text)
                    }
                }
            }

            Item {
                height: 50
                Layout.fillWidth: true
                CoreText {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    color: Theme.color.neutral7
                    text: "Your name"
                    font.pixelSize: 15
                }

                TextField {
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    leftPadding: 0
                    font.family: "Inter"
                    font.styleName: "Regular"
                    font.pixelSize: 18
                    color: Theme.color.neutral9
                    placeholderTextColor: Theme.color.neutral7
                    background: Item {}
                    placeholderText: qsTr("Enter name...")
                }
            }

            Item {
                height: 50
                Layout.fillWidth: true
                CoreText {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    color: Theme.color.neutral7
                    text: "Message"
                    font.pixelSize: 15
                }

                TextField {
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    leftPadding: 0
                    font.family: "Inter"
                    font.styleName: "Regular"
                    font.pixelSize: 18
                    color: Theme.color.neutral9
                    placeholderTextColor: Theme.color.neutral7
                    background: Item {}
                    placeholderText: qsTr("Enter message...")
                }
            }
        }
    }
}
