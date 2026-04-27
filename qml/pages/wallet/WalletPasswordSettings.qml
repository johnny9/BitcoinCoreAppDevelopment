// Copyright (c) 2026 The Bitcoin Core developers
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
    objectName: "walletPasswordSettingsPage"

    required property bool updating
    property WalletQmlModel wallet: walletController.selectedWallet
    property string errorText: ""
    property bool closingForDeselection: false

    signal back()
    signal saved()

    background: null

    header: NavigationBar2 {
        leftItem: NavButton {
            objectName: "walletPasswordBackButton"
            iconSource: "image://images/caret-left"
            text: qsTr("Back")
            onClicked: root.back()
        }
        centerItem: Header {
            objectName: "walletPasswordHeader"
            headerBold: true
            headerSize: 18
            header: root.updating ? qsTr("Update password") : qsTr("Set password")
        }
    }

    Component.onCompleted: {
        if (root.wallet) root.wallet.clearSettingsError()
        root.errorText = ""
    }

    Connections {
        target: root.wallet
        function onSettingsErrorChanged() {
            root.errorText = root.wallet ? root.wallet.settingsError : ""
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
            objectName: "walletPasswordIntroText"
            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            font.pixelSize: 15
            color: Theme.color.neutral7
            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WordWrap
            text: root.updating
                ? qsTr("Enter your current password, then choose a new password for this wallet.\n\nA secure password consists of ten or more random characters, or eight or more words.")
                : qsTr("The password is used to encrypt the wallet on your hard drive, preventing unwanted access from others.\n\nA secure password consists of ten or more random characters, or eight or more words.")
        }

        CoreText {
            objectName: "walletPasswordCurrentLabel"
            visible: root.updating
            Layout.topMargin: 30
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            font.pixelSize: 15
            color: Theme.color.neutral7
            horizontalAlignment: Text.AlignLeft
            text: qsTr("Current password")
        }

        CoreTextField {
            id: currentPassword
            objectName: "walletPasswordCurrentField"
            visible: root.updating
            Layout.fillWidth: true
            Layout.topMargin: root.updating ? 5 : 0
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            hideText: true
            placeholderText: qsTr("Enter current password...")
            onTextEdited: root.errorText = ""
        }

        CoreText {
            objectName: "walletPasswordNewLabel"
            Layout.topMargin: 30
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            font.pixelSize: 15
            color: Theme.color.neutral7
            horizontalAlignment: Text.AlignLeft
            text: root.updating ? qsTr("New password") : qsTr("Choose a password")
        }

        CoreTextField {
            id: newPassword
            objectName: "walletPasswordNewField"
            Layout.fillWidth: true
            Layout.topMargin: 5
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            focus: true
            hideText: true
            placeholderText: root.updating ? qsTr("Enter new password...") : qsTr("Enter password...")
            onTextEdited: root.errorText = ""
        }

        CoreText {
            objectName: "walletPasswordConfirmLabel"
            Layout.topMargin: 20
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            font.pixelSize: 15
            color: Theme.color.neutral7
            horizontalAlignment: Text.AlignLeft
            text: root.updating ? qsTr("Confirm new password") : qsTr("Confirm password")
        }

        CoreTextField {
            id: confirmPassword
            objectName: "walletPasswordConfirmField"
            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            hideText: true
            placeholderText: root.updating ? qsTr("Enter new password again...") : qsTr("Enter password again...")
            onTextEdited: root.errorText = ""
        }

        Setting {
            id: acknowledgement
            objectName: "walletPasswordConfirmToggle"
            visible: !root.updating
            Layout.fillWidth: true
            Layout.topMargin: 20
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            description: qsTr("I understand that if I lose or forget this password I might lose access to the bitcoin stored in this wallet.")
            actionItem: OptionSwitch { }
            onClicked: {
                loadedItem.toggle()
                loadedItem.toggled()
            }
        }

        ContinueButton {
            objectName: "walletPasswordSaveButton"
            Layout.preferredWidth: Math.min(300, parent.width - 40)
            Layout.topMargin: 40
            Layout.alignment: Qt.AlignCenter
            text: root.updating ? qsTr("Update password") : qsTr("Set password")
            enabled: walletController.initialized &&
                     newPassword.text !== "" &&
                     confirmPassword.text !== "" &&
                     newPassword.text === confirmPassword.text &&
                     (root.updating ? currentPassword.text !== "" : acknowledgement.loadedItem.checked)
            onClicked: {
                root.errorText = ""
                const ok = root.updating
                    ? root.wallet.changeWalletPassphrase(currentPassword.text, newPassword.text)
                    : root.wallet.encryptWallet(newPassword.text)
                if (ok) {
                    currentPassword.text = ""
                    newPassword.text = ""
                    confirmPassword.text = ""
                    if (!root.updating && acknowledgement.loadedItem) {
                        acknowledgement.loadedItem.checked = false
                    }
                    root.saved()
                } else {
                    root.errorText = root.wallet ? root.wallet.settingsError : ""
                }
            }
        }

        CoreText {
            objectName: "walletPasswordErrorText"
            Layout.fillWidth: true
            Layout.topMargin: 20
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            visible: text.length > 0
            text: root.errorText
            color: Theme.color.red
            font.pixelSize: 15
            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WordWrap
        }
    }
}
