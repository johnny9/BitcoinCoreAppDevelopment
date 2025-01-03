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
    property string message: ""
    property string amount: ""
    property string label: ""
    property string address: ""
    property string direction: ""
    property string status: ""
    property int depth: 0;

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
                text: qsTr("Transaction")
                font.pixelSize: 18
                bold: true
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
            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(parent.width, 450)
            spacing: 0

            Rectangle {
                Layout.topMargin: 25
                Layout.bottomMargin: 25
                width: 60
                height: 60
                Layout.alignment: Qt.AlignHCenter
                radius: 30
                color: Theme.color.green

                Icon {
                    anchors.centerIn: parent
                    source: "qrc:/icons/triangle-down"
                    color: Theme.color.white
                    size: 30
                }
            }

            CoreText {
                Layout.alignment: Qt.AlignHCenter
                Layout.bottomMargin: 5
                text: root.amount
                color: Theme.color.green
                font.pixelSize: 28
            }

            CoreText {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Created just now")
                color: Theme.color.neutral7
                font.pixelSize: 18
            }

            CoreText {
                Layout.alignment: Qt.AlignHCenter
                Layout.bottomMargin: 10
                text: qsTr("%1 confirmations").arg(root.depth)
                color: Theme.color.neutral7
                font.pixelSize: 18
            }

            LabeledTextInput {
                id: labelTextInput
                Layout.fillWidth: true
                Layout.bottomMargin: 20
                labelText: qsTr("Label")
                visible: root.label != ""
                enabled: false
                text: root.label
            }

            LabeledTextInput {
                id: messageTextInput
                Layout.fillWidth: true
                Layout.bottomMargin: 20
                labelText: qsTr("Message")
                visible: root.message != ""
                enabled: false
                text: root.message
            }

            Item {
                height: addressLabel.height + addressText.height
                Layout.fillWidth: true
                Layout.bottomMargin: 20
                CoreText {
                    id: addressLabel
                    anchors.left: parent.left
                    anchors.top: parent.top
                    color: Theme.color.neutral7
                    text: qsTr("Address")
                    font.pixelSize: 15
                }

                CoreText {
                    id: addressText
                    anchors.left: parent.left
                    anchors.right: copyIcon.left
                    anchors.top: addressLabel.bottom
                    leftPadding: 0
                    font.family: "Inter"
                    font.styleName: "Regular"
                    font.pixelSize: 18
                    horizontalAlignment: Text.AlignLeft
                    color: Theme.color.neutral9
                    text: "bc1q wvlv mha3 cvhy q6qz tjzu mq2d 63ff htzy xxu6 q8"
                }

                Icon {
                    id: copyIcon
                    anchors.right: parent.right
                    anchors.verticalCenter: addressText.verticalCenter
                    source: "image://images/copy"
                    color: Theme.color.neutral8
                    size: 30
                    enabled: true
                    onClicked: {
                        Clipboard.setText(addressText.text)
                    }
                }
            }
        }
    }
}
