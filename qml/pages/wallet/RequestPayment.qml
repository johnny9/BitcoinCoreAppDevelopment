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
    id: root
    background: null

    property WalletQmlModel wallet: walletController.selectedWallet
    property PaymentRequest request: wallet ? wallet.currentPaymentRequest : null

    Binding {
        target: root.request ? root.request.amount : null
        property: "unit"
        value: optionsModel.displayUnit
    }

    ScrollView {
        clip: true
        width: parent.width
        height: parent.height
        contentWidth: width

        CoreText {
            id: title
            anchors.left: contentRow.left
            anchors.top: parent.top
            anchors.topMargin: 20
            text: root.request !== null && root.request.id !== ""
                ? qsTr("Payment request #") + root.request.id
                : qsTr("Request a payment")
            font.pixelSize: 21
            bold: true
        }

        RowLayout {
            id: contentRow

            enabled: walletController.initialized && root.request !== null

            anchors.top: title.bottom
            anchors.topMargin: 40
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 30
            ColumnLayout {
                id: columnLayout
                Layout.minimumWidth: 450
                Layout.maximumWidth: 470

                spacing: 5

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
                        font.family: "BitcoinCoreSans"
                        font.styleName: "Regular"
                        font.pixelSize: 18
                        color: Theme.color.neutral9
                        placeholderTextColor: enabled ? Theme.color.neutral7 : Theme.color.neutral4
                        background: Item {}
                        placeholderText: root.request && root.request.amount.unit === BitcoinAmount.SAT ? "0" : "0.00000000"
                        selectByMouse: true
                        text: root.request ? root.request.amount.display : ""
                        onTextEdited: {
                            if (root.request) {
                                root.request.amount.display = text
                            }
                        }
                        onEditingFinished: {
                            if (root.request) {
                                root.request.amount.format()
                            }
                        }
                        onActiveFocusChanged: {
                            if (!activeFocus && root.request) {
                                root.request.amount.format()
                            }
                        }
                        validator: RegularExpressionValidator {
                            regularExpression: !root.request || root.request.amount.unit === BitcoinAmount.BTC
                                ? /^(0|[1-9]\d{0,7})(\.\d{0,8})?$/
                                : /^(0|[1-9]\d{0,15})$/
                        }
                        maximumLength: !root.request || root.request.amount.unit === BitcoinAmount.BTC ? 17 : 16
                    }
                    Item {
                        width: unitLabel.width + flipIcon.width
                        height: Math.max(unitLabel.height, flipIcon.height)
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (root.request) {
                                    root.request.amount.flipUnit()
                                }
                            }
                        }
                        CoreText {
                            id: unitLabel
                            anchors.right: flipIcon.left
                            anchors.verticalCenter: parent.verticalCenter
                            text: root.request ? root.request.amount.unitLabel : ""
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
                    visible: root.request !== null && root.request.amountError.length > 0

                    Icon {
                        source: "image://images/alert-filled"
                        size: 22
                        color: Theme.color.red
                    }

                    CoreText {
                        text: root.request ? root.request.amountError : ""
                        font.pixelSize: 15
                        color: Theme.color.red
                        horizontalAlignment: Text.AlignLeft
                        Layout.fillWidth: true
                    }
                }

                Separator {
                    Layout.fillWidth: true
                }

                LabeledTextInput {
                    id: label
                    Layout.fillWidth: true
                    labelText: qsTr("Label")
                    placeholderText: qsTr("Enter label...")
                    text: root.request ? root.request.label : ""
                    onTextEdited: {
                        if (root.request) {
                            root.request.label = label.text
                        }
                    }
                }

                Separator {
                    Layout.fillWidth: true
                }

                LabeledTextInput {
                    id: message
                    Layout.fillWidth: true
                    labelText: qsTr("Message")
                    placeholderText: qsTr("Enter message...")
                    text: root.request ? root.request.message : ""
                    onTextEdited: {
                        if (root.request) {
                            root.request.message = message.text
                        }
                    }
                }

                Separator {
                    Layout.fillWidth: true
                }

                Item {
                    Layout.fillWidth: true
                    Layout.minimumHeight: addressLabel.height + copyLabel.height
                    Layout.topMargin: 10
                    height: addressLabel.height + copyLabel.height
                    CoreText {
                        id: addressLabel
                        anchors.left: parent.left
                        anchors.top: parent.top
                        horizontalAlignment: Text.AlignLeft
                        width: 110
                        text: qsTr("Address")
                        font.pixelSize: 18
                    }
                    CoreText {
                        id: copyLabel
                        anchors.left: parent.left
                        anchors.top: addressLabel.bottom
                        horizontalAlignment: Text.AlignLeft
                        width: 110
                        text: qsTr("copy")
                        font.pixelSize: 18
                        color: copyArea.enabled ? Theme.color.orange : Theme.color.neutral4
                    }

                    Rectangle {
                        anchors.left: addressLabel.right
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        color: Theme.color.neutral2
                        radius: 5
                        CoreText {
                            id: address
                            anchors.fill: parent
                            anchors.leftMargin: 5
                            horizontalAlignment: Text.AlignLeft
                            font.family: "Roboto Mono"
                            font.styleName: "Regular"
                            font.pixelSize: 18
                            wrapMode: Text.WordWrap
                            text: root.request ? root.request.addressFormatted : ""
                        }
                    }

                    MouseArea {
                        id: copyArea
                        anchors.left: parent.left
                        anchors.top: addressLabel.bottom
                        anchors.right: addressLabel.right
                        anchors.bottom: parent.bottom
                        hoverEnabled: true
                        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                        enabled: root.request !== null && root.request.address !== ""
                        onClicked: Clipboard.setText(root.request.address)
                    }

                    MouseArea {
                        id: addressArea
                        anchors.left: addressLabel.right
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        hoverEnabled: true
                        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                        enabled: root.request !== null && root.request.address !== ""
                        onClicked: Clipboard.setText(root.request.address)
                    }
                }

                ContinueButton {
                    id: continueButton
                    Layout.fillWidth: true
                    Layout.topMargin: 30
                    text: root.request !== null && root.request.id !== ""
                        ? qsTr("Copy payment request")
                        : qsTr("Create bitcoin address")
                    onClicked: {
                        if (!root.request) {
                            return
                        }
                        if (root.request.address === "") {
                            root.wallet.commitPaymentRequest()
                        } else {
                            Clipboard.setText(root.request.address)
                        }
                    }
                }

                ContinueButton {
                    id: clearRequest
                    Layout.fillWidth: true
                    Layout.topMargin: 10
                    visible: root.request !== null && root.request.id !== ""
                    borderColor: Theme.color.neutral6
                    borderHoverColor: Theme.color.orangeLight1
                    borderPressedColor: Theme.color.orangeLight2
                    backgroundColor: "transparent"
                    backgroundHoverColor: "transparent"
                    backgroundPressedColor: "transparent"
                    text: qsTr("Clear")
                    onClicked: {
                        if (root.request) {
                            root.request.clear()
                        }
                    }
                }

                Connections {
                    target: walletController
                    function onSelectedWalletChanged() {
                        if (root.request) {
                            root.request.clear()
                        }
                    }
                }
            }

            Pane {
                Layout.alignment: Qt.AlignTop
                Layout.minimumWidth: 150
                Layout.minimumHeight: 150
                padding: 0
                background: Rectangle {
                    color: Theme.color.neutral2
                    visible: qrImage.code === ""
                }
                contentItem: QRImage {
                    id: qrImage
                    backgroundColor: "transparent"
                    foregroundColor: Theme.color.neutral9
                    code: root.request ? root.request.address : ""
                }
            }
        }
    }
}
