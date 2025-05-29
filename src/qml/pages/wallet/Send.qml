// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.settings 1.0
import org.bitcoincore.qt 1.0

import "../../controls"
import "../../components"

PageStack {
    id: root

    vertical: true

    property WalletQmlModel wallet: walletController.selectedWallet
    property SendRecipient recipient: wallet.recipients.current

    signal transactionPrepared(bool multipleRecipientsEnabled)

    Connections {
        target: walletController
        function onSelectedWalletChanged() {
            root.pop()
        }
    }

    initialItem: Page {
        background: null

        Settings {
            id: settings

            property alias coinControlEnabled: sendOptionsPopup.coinControlEnabled
            property alias multipleRecipientsEnabled: sendOptionsPopup.multipleRecipientsEnabled
        }

        ScrollView {
            clip: true
            width: parent.width
            height: parent.height
            contentWidth: width // Typically same as width for vertical scroll

            ColumnLayout {
                id: columnLayout

                width: 450 // Consider Layout.preferredWidth if parent was a Layout
                anchors.horizontalCenter: parent.horizontalCenter

                spacing: 10
                enabled: walletController.initialized

                Item { // Title Section
                    id: titleRow

                    Layout.fillWidth: true
                    Layout.topMargin: 30
                    Layout.bottomMargin: 20

                    CoreText {
                        id: title

                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter

                        text: qsTr("Send bitcoin")
                        font.pixelSize: 21
                        color: Theme.color.neutral9
                        bold: true
                    }

                    EllipsisMenuButton {
                        id: menuButton

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter

                        checked: sendOptionsPopup.opened
                        onClicked: sendOptionsPopup.open()
                    }

                    SendOptionsPopup {
                        id: sendOptionsPopup

                        x: menuButton.x - width + menuButton.width
                        y: menuButton.y + menuButton.height
                    }
                } // End Title Section

                RowLayout { // Recipient Navigation (Multiple Recipients)
                    id: selectAndAddRecipients

                    Layout.fillWidth: true
                    Layout.topMargin: 10
                    Layout.bottomMargin: 10

                    visible: settings.multipleRecipientsEnabled

                    NavButton {
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30

                        iconWidth: 30
                        iconHeight: 30
                        iconSource: "image://images/caret-left"
                        onClicked: wallet.recipients.prev()
                    }

                    NavButton {
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30

                        iconWidth: 30
                        iconHeight: 30
                        iconSource: "image://images/caret-right"
                        onClicked: wallet.recipients.next()
                    }

                    CoreText {
                        id: selectAndAddRecipientsLabel

                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignLeft // For CoreText in a RowLayout, AlignVCenter might also be desired depending on other items

                        text: qsTr("Recipient %1 of %2").arg(wallet.recipients.currentIndex).arg(wallet.recipients.count)
                        font.pixelSize: 18
                    }

                    NavButton {
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30

                        iconWidth: 20
                        iconHeight: 20
                        iconSource: "image://images/plus"
                        onClicked: wallet.recipients.add()
                    }

                    NavButton {
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30

                        iconWidth: 20
                        iconHeight: 20
                        iconSource: "image://images/minus"
                        visible: wallet.recipients.count > 1
                        onClicked: wallet.recipients.remove()
                    }
                } // End Recipient Navigation

                Separator {
                    Layout.fillWidth: true
                    visible: settings.multipleRecipientsEnabled
                }

                LabeledTextInput { // Address Input
                    id: address

                    Layout.fillWidth: true

                    labelText: qsTr("Send to")
                    placeholderText: qsTr("Enter address...")
                    text: root.recipient.address
                    onTextEdited: root.recipient.address = address.text
                }

                Separator {
                    Layout.fillWidth: true
                }

                RowLayout { // Amount Input Section
                    Layout.fillWidth: true

                    BitcoinAmount { // Non-visual component, order less critical but often at top
                        id: bitcoinAmount
                    }

                    CoreText {
                        id: amountLabel

                        Layout.alignment: Qt.AlignVCenter

                        horizontalAlignment: Text.AlignLeft
                        text: qsTr("Amount")
                        font.pixelSize: 18
                    }

                    TextField {
                        id: amountInput

                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter

                        leftPadding: 0
                        font.family: "Inter"
                        font.styleName: "Regular"
                        font.pixelSize: 18
                        placeholderTextColor: enabled ? Theme.color.neutral7 : Theme.color.neutral2
                        background: Item {} // Empty background, standard for custom text fields
                        placeholderText: "0.00000000"
                        selectByMouse: true
                        onTextEdited: {
                            amountInput.text = bitcoinAmount.amount = bitcoinAmount.sanitize(amountInput.text)
                            root.recipient.amount = bitcoinAmount.satoshiAmount
                        }
                    }

                    Item { // Unit Switcher
                        Layout.alignment: Qt.AlignVCenter

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (bitcoinAmount.unit == BitcoinAmount.BTC) {
                                    amountInput.text = bitcoinAmount.convert(amountInput.text, BitcoinAmount.BTC)
                                    bitcoinAmount.unit = BitcoinAmount.SAT
                                } else {
                                    amountInput.text = bitcoinAmount.convert(amountInput.text, BitcoinAmount.SAT)
                                    bitcoinAmount.unit = BitcoinAmount.BTC
                                }
                            }
                        }

                        CoreText {
                            id: unitLabel

                            anchors.right: flipIcon.left
                            anchors.verticalCenter: parent.verticalCenter

                            text: bitcoinAmount.unitLabel
                            font.pixelSize: 18
                            color: enabled ? Theme.color.neutral7 : Theme.color.neutral2
                        }

                        Icon {
                            id: flipIcon

                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter

                            source: "image://images/flip-vertical"
                            color: enabled ? Theme.color.neutral8 : Theme.color.neutral2
                            size: 30
                        }
                    }
                } // End Amount Input Section

                Separator {
                    Layout.fillWidth: true
                }

                LabeledTextInput { // Label Input
                    id: label

                    Layout.fillWidth: true

                    labelText: qsTr("Note to self")
                    placeholderText: qsTr("Enter ...")
                    onTextEdited: root.recipient.label = label.text
                }

                Separator {
                    Layout.fillWidth: true
                }

                LabeledCoinControlButton { // Coin Control
                    id: coinControlButton // Added id for consistency, though not strictly necessary if not referenced
                    Layout.fillWidth: true

                    visible: settings.coinControlEnabled
                    coinsSelected: wallet.coinsListModel.selectedCoinsCount
                    coinCount: wallet.coinsListModel.coinCount
                    onOpenCoinControl: {
                        root.wallet.coinsListModel.update()
                        root.push(coinSelectionPage)
                    }
                }

                Separator {
                    Layout.fillWidth: true
                    visible: settings.coinControlEnabled
                }

                FeeSelection { // Fee Selection
                    id: feeSelection

                    Layout.fillWidth: true
                    Layout.preferredHeight: 50

                    onSelectedLabelChanged: {
                        // Original logic was commented out, preserving that state.
                    }
                }

                ContinueButton { // Continue Button
                    id: continueButton

                    Layout.fillWidth: true
                    Layout.topMargin: 30

                    text: qsTr("Review")
                    onClicked: {
                        if (root.wallet.prepareTransaction()) {
                            root.transactionPrepared(settings.multipleRecipientsEnabled);
                        }
                    }
                }
            } // End ColumnLayout
        } // End ScrollView
    } // End initialItem Page

    Component { // Coin Selection Page Component
        id: coinSelectionPage

        CoinSelection {
            // Properties of CoinSelection would go here if needed
            onDone: root.pop()
        }
    }
} // End PageStack
