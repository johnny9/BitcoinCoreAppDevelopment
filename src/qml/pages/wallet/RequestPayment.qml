// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../../controls"
import "../../components"
import "../settings"

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
                text: qsTr("All fields are optional. Information is only knowable by the person you share it with.")
            }

            Item {
                height: 50
                Layout.fillWidth: true
                CoreText {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    color: Theme.color.neutral7
                    text: "Note to self"
                    font.pixelSize: 21
                }

                TextInput {
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    selectByMouse: true
                    font.family: "Inter"
                    font.styleName: "Regular"
                    font.pointSize: 18
                    color: Theme.color.neutral9
                    text: "Enter note..."
                }
            }

            Item {
                height: 50
                Layout.fillWidth: true
                CoreText {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    color: Theme.color.neutral7
                    text: "Amount"
                    font.pixelSize: 21
                }

                TextInput {
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    font.family: "Inter"
                    font.styleName: "Regular"
                    font.pointSize: 18
                    color: Theme.color.neutral9
                    text: "0.00000000"
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
                    font.pixelSize: 21
                }

                TextInput {
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    font.family: "Inter"
                    font.styleName: "Regular"
                    font.pointSize: 18
                    color: Theme.color.neutral9
                    text: "Enter name..."
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
                    font.pixelSize: 21
                }

                TextInput {
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    font.family: "Inter"
                    font.styleName: "Regular"
                    font.pointSize: 18
                    color: Theme.color.neutral9
                    text: "Enter message..."
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
                }
            }
        }
    }
}