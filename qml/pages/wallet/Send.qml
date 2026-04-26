// Copyright (c) 2024-2026 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs
import Qt.labs.settings 1.0
import org.bitcoincore.qt 1.0

import "../../controls"
import "../../components"

PageStack {
    id: root
    objectName: "sendPage"
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

    Connections {
        target: root.wallet.recipients
        function onListCleared() {
            settings.multipleRecipientsEnabled = false
        }
    }


    initialItem: Page {
        id: sendPage
        background: null

        function handlePsbtImportResult(mode) {
            sendOptionsPopup.close()
            if (mode === "single-review" || mode === "multiple-review") {
                const multipleRecipientsEnabled = mode === "multiple-review"
                settings.multipleRecipientsEnabled = multipleRecipientsEnabled
                root.transactionPrepared(multipleRecipientsEnabled)
            } else if (mode === "unsupported") {
                unsupportedPsbtPopup.message = root.wallet.importedPsbtError.length > 0
                    ? root.wallet.importedPsbtError
                    : qsTr("This PSBT is not supported yet.")
                unsupportedPsbtPopup.open()
            }
        }

        FileDialog {
            id: psbtOpenDialog
            title: qsTr("Import PSBT")
            fileMode: FileDialog.OpenFile
            nameFilters: [qsTr("Partially Signed Bitcoin Transactions (*.psbt)"), qsTr("All files (*)")]
            onAccepted: sendPage.handlePsbtImportResult(root.wallet.importPsbtFromFile(selectedFile.toString()))
        }

        // Kept hidden so functional tests can inject a PSBT path until the
        // native file dialog is automatable through the QML test bridge.
        TextField {
            id: psbtAutomationPathField
            objectName: "psbtImportPathField"
            visible: false
        }

        Popup {
            id: unsupportedPsbtPopup
            objectName: "unsupportedPsbtPopup"
            anchors.centerIn: parent
            modal: true
            padding: 20
            width: Math.min(parent.width - 40, 360)

            property string message: ""

            onClosed: root.wallet.clearImportedPsbt()

            background: Rectangle {
                color: Theme.color.background
                radius: 8
                border.color: Theme.color.neutral3
                border.width: 1
            }

            ColumnLayout {
                width: parent.width
                spacing: 16

                CoreText {
                    objectName: "unsupportedPsbtPopupTitle"
                    Layout.fillWidth: true
                    text: qsTr("Not supported")
                    bold: true
                    font.pixelSize: 18
                    color: Theme.color.neutral9
                    horizontalAlignment: Text.AlignHCenter
                }

                CoreText {
                    objectName: "unsupportedPsbtPopupMessage"
                    Layout.fillWidth: true
                    text: unsupportedPsbtPopup.message
                    font.pixelSize: 15
                    color: Theme.color.neutral7
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                }

                ContinueButton {
                    objectName: "unsupportedPsbtPopupOkButton"
                    Layout.fillWidth: true
                    text: qsTr("Ok")
                    onClicked: unsupportedPsbtPopup.close()
                }
            }
        }

        Settings {
            id: settings
            property alias coinControlEnabled: sendOptionsPopup.coinControlEnabled
            property alias multipleRecipientsEnabled: sendOptionsPopup.multipleRecipientsEnabled

            onMultipleRecipientsEnabledChanged: {
                if (!multipleRecipientsEnabled) {
                    root.wallet.recipients.clearToFront()
                } else if (root.wallet.recipients.count === 1) {
                    root.wallet.recipients.add()
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

                spacing: 10

                enabled: walletController.initialized

                Item {
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

                    IconButton {
                        id: menuButton
                        objectName: "sendOptionsMenuButton"
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        checked: sendOptionsPopup.opened
                        iconSource: "image://images/ellipsis"
                        onClicked: {
                            sendOptionsPopup.open()
                        }
                    }

                    SendOptionsPopup {
                        id: sendOptionsPopup
                        x: menuButton.x - width + menuButton.width
                        y: menuButton.y + menuButton.height
                        onImportPsbtFromFileRequested: {
                            if (psbtAutomationPathField.text.length > 0) {
                                const automatedPath = psbtAutomationPathField.text
                                psbtAutomationPathField.text = ""
                                sendPage.handlePsbtImportResult(root.wallet.importPsbtFromFile(automatedPath))
                                return
                            }
                            sendOptionsPopup.close()
                            psbtOpenDialog.open()
                        }
                    }
                }

                RowLayout {
                    id: selectAndAddRecipients
                    Layout.fillWidth: true
                    Layout.topMargin: 10
                    Layout.bottomMargin: 10
                    visible: settings.multipleRecipientsEnabled

                    CoreText {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignLeft
                        id: selectAndAddRecipientsLabel
                        text: qsTr("Recipient %1 of %2").arg(wallet.recipients.currentIndex).arg(wallet.recipients.count)
                        horizontalAlignment: Text.AlignLeft
                        font.pixelSize: 18
                        color: Theme.color.neutral9
                    }

                    IconButton {
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30
                        size: 30
                        iconSource: "image://images/caret-left"
                        enabled: wallet.recipients.currentIndex - 1 > 0
                        onClicked: {
                            wallet.recipients.prev()
                        }
                    }

                    IconButton {
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30
                        size: 30
                        iconSource: "image://images/caret-right"
                        enabled: wallet.recipients.currentIndex < wallet.recipients.count
                        onClicked: {
                            wallet.recipients.next()
                        }
                    }

                    IconButton {
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30
                        size: 30
                        iconSource: "image://images/plus-big-filled"
                        enabled: wallet.recipients.count < 25
                        onClicked: {
                            wallet.recipients.add()
                        }
                    }

                    IconButton {
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30
                        size: 30
                        iconSource: "image://images/minus"
                        enabled: wallet.recipients.count > 1
                        onClicked: {
                            wallet.recipients.remove()
                        }
                    }
                }

                Separator {
                    visible: settings.multipleRecipientsEnabled
                    Layout.fillWidth: true
                }

                BitcoinAddressInputField {
                    Layout.fillWidth: true
                    enabled: walletController.initialized
                    address: root.recipient.address
                    errorText: root.recipient.addressError
                }

                Separator {
                    Layout.fillWidth: true
                }

                ColumnLayout {
                    Layout.fillWidth: true

                    Item {
                        height: amountInput.height
                        Layout.fillWidth: true
                        CoreText {
                            id: amountLabel
                            width: 110
                            anchors.left: parent.left
                            anchors.verticalCenter: parent.verticalCenter
                            horizontalAlignment: Text.AlignLeft
                            text: qsTr("Amount")
                            font.pixelSize: 18
                        }

                        TextField {
                            id: amountInput
                            anchors.left: amountLabel.right
                            anchors.verticalCenter: parent.verticalCenter
                            leftPadding: 0
                            font.family: "Inter"
                            font.styleName: "Regular"
                            font.pixelSize: 18
                            color: Theme.color.neutral9
                            placeholderTextColor: enabled ? Theme.color.neutral7 : Theme.color.neutral4
                            background: Item {}
                            placeholderText: "0.00000000"
                            selectByMouse: true
                            text: root.recipient.amount.display
                            onTextEdited: root.recipient.amount.display = text
                            onEditingFinished: root.recipient.amount.format()
                            onActiveFocusChanged: {
                                if (!activeFocus) {
                                    root.recipient.amount.format()
                                }
                            }
                            validator: RegularExpressionValidator {
                                regularExpression: root.recipient.amount.unit === BitcoinAmount.BTC
                                    ? /^(0|[1-9]\d{0,7})(\.\d{0,8})?$/
                                    : /^(0|[1-9]\d{0,15})$/
                            }
                            maximumLength: root.recipient.amount.unit === BitcoinAmount.BTC ? 17 : 16
                        }
                        Item {
                            width: unitLabel.width + flipIcon.width
                            height: Math.max(unitLabel.height, flipIcon.height)
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            MouseArea {
                                anchors.fill: parent
                                onClicked: root.recipient.amount.flipUnit()
                            }
                            CoreText {
                                id: unitLabel
                                anchors.right: flipIcon.left
                                anchors.verticalCenter: parent.verticalCenter
                                text: root.recipient.amount.unitLabel
                                font.pixelSize: 18
                                color: enabled ? Theme.color.neutral7 : Theme.color.neutral4
                            }
                            Icon {
                                id: flipIcon
                                anchors.right: parent.right
                                anchors.verticalCenter: parent.verticalCenter
                                source: "image://images/flip-vertical"
                                color: unitLabel.enabled ? Theme.color.neutral8 : Theme.color.neutral4
                                size: 30
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        visible: root.recipient.amountError.length > 0

                        Icon {
                            source: "image://images/alert-filled"
                            size: 22
                            color: Theme.color.red
                        }

                        CoreText {
                            text: root.recipient.amountError
                            font.pixelSize: 15
                            color: Theme.color.red
                            horizontalAlignment: Text.AlignLeft
                            Layout.fillWidth: true
                        }
                    }
                }

                Separator {
                    Layout.fillWidth: true
                }

                LabeledTextInput {
                    id: label
                    Layout.fillWidth: true
                    labelText: qsTr("Note to self")
                    placeholderText: qsTr("Enter ...")
                    text: root.recipient.label
                    onTextEdited: root.recipient.label = label.text
                }

                Separator {
                    Layout.fillWidth: true
                }

                LabeledCoinControlButton {
                    visible: settings.coinControlEnabled
                    Layout.fillWidth: true
                    coinsSelected: wallet.coinsListModel.selectedCoinsCount
                    coinCount: wallet.coinsListModel.coinCount
                    onOpenCoinControl: {
                        root.wallet.coinsListModel.update()
                        root.push(coinSelectionPage)
                    }
                }

                Separator {
                    visible: settings.coinControlEnabled
                    Layout.fillWidth: true
                }

                FeeSelection {
                    id: feeSelection
                    Layout.fillWidth: true

                    onFeeChanged: {
                        root.wallet.targetBlocks = target
                    }
                }

                ContinueButton {
                    id: continueButton
                    Layout.fillWidth: true
                    Layout.topMargin: 30
                    text: qsTr("Review")
                    enabled: root.recipient.isValid
                    onClicked: {
                        if (root.wallet.prepareTransaction()) {
                            root.transactionPrepared(settings.multipleRecipientsEnabled);
                        }
                    }
                }
            }
        }
    }

    Component {
        id: coinSelectionPage
        CoinSelection {
            onDone: root.pop()
        }
    }
}
