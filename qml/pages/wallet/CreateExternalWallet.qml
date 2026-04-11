// Copyright (c) 2026 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../../controls"
import "../../components"

Page {
    id: root
    signal back
    signal next

    property string defaultWalletName: ""
    readonly property bool creatingWallet: walletController.walletLoadInProgress

    background: null

    Component.onCompleted: {
        walletController.clearWalletLoadStatus()
        walletController.refreshExternalSignerStatus()
    }
    onVisibleChanged: {
        if (visible) {
            walletController.refreshExternalSignerStatus()
        }
    }

    header: NavigationBar2 {
        leftItem: NavButton {
            iconSource: "image://images/caret-left"
            text: qsTr("Back")
            enabled: !root.creatingWallet
            onClicked: root.back()
        }
    }

    Connections {
        target: walletController
        function onWalletLoadSucceeded() {
            if (root.visible) {
                root.next()
            }
        }
    }

    ColumnLayout {
        id: columnLayout
        width: Math.min(parent.width, 450)
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 24

        Header {
            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            header: qsTr("Create an external wallet")
            headerBold: true
            description: walletController.externalSignerError.length > 0
                ? walletController.externalSignerError
                : walletController.externalSignerName.length > 0
                ? qsTr("Connected signer: %1").arg(walletController.externalSignerName)
                : qsTr("Connect one external signer to continue.")
        }

        CoreText {
            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            font.pixelSize: 15
            color: Theme.color.neutral9
            wrapMode: Text.WordWrap
            text: qsTr("This wallet stores public descriptors and uses your external signer device for signing.")
        }

        CoreTextField {
            id: walletNameInput
            objectName: "externalWalletNameInput"
            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            focus: !root.creatingWallet
            enabled: !root.creatingWallet
            placeholderText: qsTr("Eg. hardware_wallet")
            text: root.defaultWalletName
            validator: RegularExpressionValidator { regularExpression: /^[a-zA-Z0-9_]{1,20}$/ }
            onTextChanged: walletController.clearWalletLoadStatus()
        }

        CoreText {
            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            visible: walletController.walletLoadError.length > 0
            color: Theme.color.red
            wrapMode: Text.WordWrap
            text: walletController.walletLoadError
        }

        RowLayout {
            objectName: "externalWalletCreationLoadingState"
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 4
            visible: root.creatingWallet
            spacing: 10

            BusyIndicator {
                objectName: "externalWalletCreationBusyIndicator"
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                running: parent.visible
            }

            CoreText {
                objectName: "externalWalletCreationStatusText"
                text: qsTr("Creating wallet with the connected signer...")
                color: Theme.color.neutral7
                font.pixelSize: 15
                wrap: false
            }
        }

        ContinueButton {
            objectName: "createExternalWalletButton"
            Layout.preferredWidth: Math.min(300, columnLayout.width - 40)
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            Layout.alignment: Qt.AlignCenter
            enabled: !root.creatingWallet && walletNameInput.acceptableInput && walletController.canCreateExternalSignerWallet
            text: root.creatingWallet ? qsTr("Creating wallet...") : qsTr("Create wallet")
            onClicked: {
                walletController.clearWalletLoadStatus()
                walletController.createExternalSignerWallet(walletNameInput.text)
            }
        }
    }
}
