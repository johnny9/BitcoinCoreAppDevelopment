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

            CoreText {
                id: title
                Layout.topMargin: 30
                Layout.bottomMargin: 20
                text: qsTr("Send bitcoin")
                font.pixelSize: 21
                bold: true
            }

            RowLayout {
                CoreText {
                    text: qsTr("Amount")
                    font.pixelSize: 15
                    color: Theme.color.neutral9
                }
                CoreText {
                    text: root.amount
                    font.pixelSize: 15
                    color: Theme.color.neutral9
                }
            }

            RowLayout {
                CoreText {
                    text: qsTr("Fee")
                    font.pixelSize: 15
                    color: Theme.color.neutral9
                }
                CoreText {
                    text: root.fee
                    font.pixelSize: 15
                    color: Theme.color.neutral9
                }
            }

            Separator {
                Layout.fillWidth: true
            }

            StackedLabeledTextInput {
                id: label
                Layout.fillWidth: true
                labelText: qsTr("Label")
                placeholderText: qsTr("")
            }

            Separator {
                Layout.fillWidth: true
            }

            ContinueButton {
                id: continueButton
                Layout.fillWidth: true
                Layout.topMargin: 30
                text: qsTr("Cancel")
            }

            ContinueButton {
                id: continueButton
                Layout.fillWidth: true
                Layout.topMargin: 30
                text: qsTr("Send")
            }
        }
    }z
}
