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

            CoreText {
                text: "Amount"
            }

            CoreTextField {
                text: "0.00 000 000"
            }

            Separator {
            }

            CoreText {
                text: "Name"
            }

            CoreTextField {
                text: "..."
            }

            Separator {
            }

            CoreText {
                text: "Message"
            }

            CoreTextField {
                text: "..."
            }

            Separator {
            }

            CoreText {
                text: "Note to self"
            }

            CoreTextField {
                text: "..."
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