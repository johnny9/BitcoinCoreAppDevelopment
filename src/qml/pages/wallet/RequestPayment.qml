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
    property PaymentRequest request: wallet.currentPaymentRequest

    ScrollView {
        clip: true
        anchors.fill: parent
        contentWidth: width

        ColumnLayout {
            id: scrollContent
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 20
            spacing: 20

            CoreText {
                id: title
                Layout.alignment: Qt.AlignLeft
                text: root.request.id === ""
                    ? qsTr("Request a payment")
                    : qsTr("Payment request #") + root.request.id
                font.pixelSize: 21
                bold: true
            }

            RowLayout {
                id: contentRow
                Layout.fillWidth: true
                enabled: walletController.initialized
                spacing: 30

                ColumnLayout {
                    id: formColumn
                    Layout.minimumWidth: 450
                    Layout.maximumWidth: 450

                    spacing: 10

                    BitcoinAmountInputField {
                        Layout.fillWidth: true
                        enabled: walletController.initialized
                        amount: root.request.amount
                        errorText: root.request.amountError
                    }

                    Separator {
                        Layout.fillWidth: true
                    }

                    LabeledTextInput {
                        id: label
                        Layout.fillWidth: true
                        labelText: qsTr("Label")
                        placeholderText: qsTr("Enter label...")
                        text: root.request.label
                        onTextEdited: root.request.label = label.text
                    }

                    Separator {
                        Layout.fillWidth: true
                    }

                    LabeledTextInput {
                        id: message
                        Layout.fillWidth: true
                        labelText: qsTr("Message")
                        placeholderText: qsTr("Enter message...")
                        text: root.request.message
                        onTextEdited: root.request.message = message.text
                    }

                    Separator {
                        Layout.fillWidth: true
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.topMargin: 10

                        spacing: 0

                        ColumnLayout {
                            spacing: 5
                            Layout.alignment: Qt.AlignLeft
                            Layout.minimumWidth: 110

                            CoreText {
                                id: addressLabel
                                text: qsTr("Address")
                                font.pixelSize: 18
                            }
                            MouseArea {
                                id: copyLabelArea
                                width: copyLabel.implicitWidth
                                height: copyLabel.implicitHeight
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                enabled: address.text !== ""

                                CoreText {
                                    id: copyLabel
                                    text: qsTr("copy")
                                    font.pixelSize: 18
                                    color: enabled ? Theme.color.orange : Theme.color.neutral4
                                }

                                onClicked: {
                                    Clipboard.setText(root.request.address)
                                }
                            }
                        }

                        Item {
                            Layout.fillWidth: true
                            Layout.minimumHeight: 100

                            MouseArea {
                                id: mouseArea
                                anchors.fill: parent
                                hoverEnabled: address.text !== ""
                                cursorShape: address.text !== "" ? Qt.PointingHandCursor : Qt.ArrowCursor
                                enabled: address.text !== ""

                                CoreText {
                                    id: address
                                    width: parent.width
                                    height: parent.height
                                    text: root.request.addressFormatted
                                    horizontalAlignment: Text.AlignLeft
                                    font.pixelSize: 18
                                    wrapMode: Text.WordWrap
                                    padding: 10
                                    background: Rectangle {
                                        color: mouseArea.containsMouse ? Theme.color.neutral3 : Theme.color.neutral2
                                        radius: 5
                                    }
                                }

                                onClicked: {
                                    Clipboard.setText(root.request.address)
                                }
                            }
                        }
                    }

                    ContinueButton {
                        id: continueButton
                        Layout.fillWidth: true
                        Layout.topMargin: 30
                        text: qsTr("Create bitcoin address")
                        onClicked: {
                            if (!clearRequest.visible) {
                                continueButton.text = qsTr("Share")
                                clearRequest.visible = true
                                root.wallet.commitPaymentRequest()
                                title.text = qsTr("Payment request #" + root.wallet.request.id)
                            }
                        }
                    }

                    ContinueButton {
                        id: clearRequest
                        Layout.fillWidth: true
                        Layout.topMargin: 10
                        visible: false
                        borderColor: Theme.color.neutral6
                        borderHoverColor: Theme.color.orangeLight1
                        borderPressedColor: Theme.color.orangeLight2
                        backgroundColor: "transparent"
                        backgroundHoverColor: "transparent"
                        backgroundPressedColor: "transparent"
                        text: qsTr("Clear")
                        onClicked: {
                            clearRequest.visible = false
                            root.request.clear()
                            continueButton.text = qsTr("Create bitcoin address")
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
                        code: root.request.address
                    }
                }
            }
        }
    }
}
