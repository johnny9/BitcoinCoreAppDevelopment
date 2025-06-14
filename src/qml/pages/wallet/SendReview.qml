// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.bitcoincore.qt 1.0

import "../../controls"
import "../../components"

Page {
    id: root
    background: null

    property WalletQmlModel wallet: walletController.selectedWallet
    property SendRecipient recipient: walletController.selectedWallet.recipients.first

    signal finished()
    signal back()
    signal transactionSent()

    header: NavigationBar2 {
        id: navbar
        leftItem: NavButton {
            iconSource: "image://images/caret-left"
            text: qsTr("Back")
            onClicked: {
                root.back()
            }
        }
    }

    ScrollView {
        clip: true
        width: parent.width
        height: parent.height
        contentWidth: width

        ColumnLayout {
            id: columnLayout
            width: 450
            anchors.horizontalCenter: parent.horizontalCenter

            spacing: 20

            CoreText {
                id: title
                Layout.topMargin: 30
                Layout.bottomMargin: 20
                text: qsTr("Transaction details")
                font.pixelSize: 25
                bold: true
            }

            RowLayout {
                CoreText {
                    text: qsTr("Send to")
                    font.pixelSize: 18
                    Layout.preferredWidth: 110
                    Layout.alignment: Qt.AlignTop
                    color: Theme.color.neutral7
                    horizontalAlignment: Text.AlignLeft
                }
                CoreText {
                    Layout.fillWidth: true
                    text: root.recipient.address.formattedAddress
                    font.pixelSize: 18
                    color: Theme.color.neutral9
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignLeft
                }
            }

            RowLayout {
                visible: root.recipient.label.length > 0
                CoreText {
                    text: qsTr("Note")
                    font.pixelSize: 18
                    Layout.preferredWidth: 110
                    Layout.alignment: Qt.AlignTop
                    color: Theme.color.neutral7
                    horizontalAlignment: Text.AlignLeft
                }
                CoreText {
                    text: root.recipient.label
                    font.pixelSize: 18
                    color: Theme.color.neutral9
                }
            }

            RowLayout {
                CoreText {
                    text: qsTr("Amount")
                    font.pixelSize: 18
                    Layout.preferredWidth: 110
                    Layout.alignment: Qt.AlignTop
                    color: Theme.color.neutral7
                    horizontalAlignment: Text.AlignLeft
                }
                CoreText {
                    Layout.preferredWidth: totalAmount.width
                    horizontalAlignment: Text.AlignRight
                    text: root.recipient.amount.richDisplay
                    font.pixelSize: 18
                    font.kerning: false
                    color: Theme.color.neutral9
                    wrap: false
                }
            }

            RowLayout {
                CoreText {
                    text: qsTr("Fee")
                    font.pixelSize: 18
                    Layout.preferredWidth: 110
                    color: Theme.color.neutral7
                    horizontalAlignment: Text.AlignLeft
                }
                CoreText {
                    Layout.preferredWidth: totalAmount.width
                    horizontalAlignment: Text.AlignRight
                    text: root.wallet.recipients.fee
                    font.pixelSize: 18
                    font.kerning: false
                    color: Theme.color.neutral9
                    wrap: false
                }
            }

            RowLayout {
                CoreText {
                    text: qsTr("Total")
                    font.pixelSize: 18
                    Layout.preferredWidth: 110
                    color: Theme.color.neutral7
                    horizontalAlignment: Text.AlignLeft
                }
                CoreText {
                    id: totalAmount
                    text: root.wallet.recipients.totalAmount
                    horizontalAlignment: Text.AlignRight
                    font.pixelSize: 18
                    font.kerning: false
                    color: Theme.color.neutral9
                    wrap: false
                }
            }

            ContinueButton {
                id: confimationButton
                Layout.fillWidth: true
                Layout.topMargin: 30
                text: qsTr("Send")
                onClicked: {
                    root.wallet.sendTransaction()
                    root.transactionSent()
                }
            }
        }
    }
}
