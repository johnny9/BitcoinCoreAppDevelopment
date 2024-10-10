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

StackView {
    id: stackView
    initialItem: pageComponent

    Component {
        id: pageComponent
        Page {
            id: root
            background: null

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

                    Item {
                        id: header
                        Layout.fillWidth: true

                        CoreText {
                            anchors.left: parent.left
                            text: qsTr("Request a payment")
                            font.pixelSize: 21
                            bold: true
                        }
                        Icon {
                            anchors.right: parent.right
                            source: "image://images/ellipsis"
                            color: Theme.color.neutral9
                            size: 30
                        }
                    }

                    CoreText {
                        text: qsTr("All fields are optional.")
                        color: Theme.color.neutral7
                        font.pixelSize: 15
                    }

                    Item {
                        height: 50
                        Layout.fillWidth: true
                        CoreText {
                            anchors.left: parent.left
                            anchors.top: parent.top
                            color: Theme.color.neutral7
                            text: "Note to self"
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
                            placeholderText: qsTr("Enter note...")
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
                            text: "Amount"
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

                    ContinueButton {
                        id: continueButton
                        Layout.preferredWidth: Math.min(300, parent.width - 2 * Layout.leftMargin)
                        Layout.leftMargin: 20
                        Layout.rightMargin: 20
                        Layout.alignment: Qt.AlignCenter
                        text: qsTr("Continue")
                        onClicked: {
                            stackView.push(Qt.resolvedUrl("RequestConfirmation.qml"))
                        }
                    }
                }
            }
        }
    }
}
