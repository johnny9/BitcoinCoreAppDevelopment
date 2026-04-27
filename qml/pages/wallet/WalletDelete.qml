// Copyright (c) 2026 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.bitcoincore.qt 1.0

import "../../controls"

Page {
    id: root
    objectName: "walletDeletePage"

    property WalletQmlModel wallet: walletController.selectedWallet
    readonly property bool confirmationMatches: wallet && wallet.displayName.length > 0 && confirmField.text === wallet.displayName
    property bool closingForDeselection: false

    signal back()
    signal deleted()

    background: null

    header: NavigationBar2 {
        leftItem: NavButton {
            objectName: "walletDeleteBackButton"
            iconSource: "image://images/caret-left"
            text: qsTr("Back")
            onClicked: root.back()
        }
        centerItem: Header {
            objectName: "walletDeleteHeader"
            headerBold: true
            headerSize: 18
            header: qsTr("Delete wallet")
        }
    }

    Connections {
        target: walletController
        function onSelectedWalletChanged() {
            if (!root.closingForDeselection &&
                (!walletController.selectedWallet || walletController.selectedWallet.name.length === 0)) {
                root.closingForDeselection = true
                Qt.callLater(root.back)
            }
        }
        function onIsWalletLoadedChanged() {
            if (!root.closingForDeselection && !walletController.isWalletLoaded) {
                root.closingForDeselection = true
                Qt.callLater(root.back)
            }
        }
    }

    ColumnLayout {
        width: Math.min(parent.width, 450)
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 30
        spacing: 0

        CoreText {
            objectName: "walletDeleteTitleText"
            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            text: qsTr("Delete this wallet from this device?")
            color: Theme.color.red
            bold: true
            font.pixelSize: 24
            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WordWrap
        }

        CoreText {
            objectName: "walletDeleteDescriptionText"
            Layout.fillWidth: true
            Layout.topMargin: 12
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            text: qsTr("This permanently removes \"%1\" from this device. If you do not have a backup or recovery information, you may lose access to the bitcoin in this wallet.").arg(root.wallet ? root.wallet.displayName : "")
            color: Theme.color.neutral8
            font.pixelSize: 15
            fontStyleName: "Regular"
            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WordWrap
        }

        CoreText {
            objectName: "walletDeleteConfirmLabel"
            Layout.topMargin: 28
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            font.pixelSize: 15
            color: Theme.color.neutral7
            horizontalAlignment: Text.AlignLeft
            text: qsTr("Type wallet name to confirm")
        }

        CoreTextField {
            id: confirmField
            objectName: "walletDeleteConfirmField"
            Layout.fillWidth: true
            Layout.topMargin: 5
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            placeholderText: qsTr("Enter wallet name...")
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 30
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            spacing: 16

            OutlineButton {
                objectName: "walletDeleteCancelButton"
                Layout.fillWidth: true
                text: qsTr("Cancel")

                HoverHandler {
                    enabled: parent.enabled && AppMode.isDesktop
                    cursorShape: Qt.PointingHandCursor
                }

                onClicked: root.back()
            }

            ContinueButton {
                objectName: "walletDeleteConfirmButton"
                Layout.fillWidth: true
                enabled: root.confirmationMatches
                text: qsTr("Delete wallet")
                textColor: Theme.color.red
                textHoverColor: Theme.color.red
                textPressedColor: Theme.color.red
                backgroundColor: Qt.rgba(Theme.color.red.r, Theme.color.red.g, Theme.color.red.b, 0.2)
                backgroundHoverColor: backgroundColor
                backgroundPressedColor: backgroundColor
                borderColor: Theme.color.red
                borderHoverColor: Theme.color.red
                borderPressedColor: Theme.color.red

                HoverHandler {
                    enabled: parent.enabled && AppMode.isDesktop
                    cursorShape: Qt.PointingHandCursor
                }

                onClicked: {
                    if (!root.wallet || !root.confirmationMatches) return
                    if (walletController.deleteWallet(root.wallet.name)) {
                        root.deleted()
                    }
                }
            }
        }
    }

    component KeyText: CoreText {
        color: Theme.color.neutral7
        font.pixelSize: 18
        fontStyleName: "Regular"
        wrap: false
        horizontalAlignment: Qt.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }

    component ValueText: CoreText {
        color: Theme.color.neutral9
        font.pixelSize: 18
        fontStyleName: "Regular"
        horizontalAlignment: Qt.AlignRight
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.WordWrap
    }
}
