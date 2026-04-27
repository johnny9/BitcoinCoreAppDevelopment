// Copyright (c) 2026 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs

import org.bitcoincore.qt 1.0

import "../../controls"

Page {
    id: root
    objectName: "walletSettingsPage"

    property WalletQmlModel wallet: walletController.selectedWallet
    property string errorText: ""
    property bool editingName: false
    property string pendingDisplayName: ""

    signal back()
    signal selectWalletRequested()
    signal passwordRequested()
    signal deleteWalletRequested()

    readonly property bool walletLoaded: walletController.isWalletLoaded

    background: null

    function backupDefaultFileUrl() {
        const wallet_name = root.wallet && root.wallet.name.length > 0
            ? root.wallet.name.replace(/[\\/]/g, "_")
            : "wallet"
        return "file://" + walletController.homePath() + "/" + wallet_name + ".bak"
    }

    function backupFileName() {
        const wallet_name = root.wallet && root.wallet.name.length > 0
            ? root.wallet.name.replace(/[\\/]/g, "_")
            : "wallet"
        return wallet_name + ".bak"
    }

    function resolvedBackupPath(rawPath) {
        let normalized = walletController.normalizeWalletPath(rawPath)
        if (normalized.length === 0) {
            return ""
        }

        const hasKnownSuffix = /\.(bak|dat)$/i.test(normalized)
        if (walletController.walletPathExists(normalized) && !hasKnownSuffix) {
            normalized = normalized + "/" + root.backupFileName()
        } else if (!hasKnownSuffix) {
            normalized = normalized + ".bak"
        }

        return normalized
    }

    function startBackup() {
        if (!root.wallet) {
            return
        }
        root.errorText = ""
        root.wallet.clearSettingsError()
        if (backupAutomationPath.text.length > 0) {
            const automatedPath = root.resolvedBackupPath(backupAutomationPath.text)
            backupAutomationPath.text = ""
            if (!root.wallet.backupWallet(automatedPath)) {
                root.errorText = root.wallet.settingsError
            }
            return
        }
        backupDialog.open()
    }

    function beginNameEdit() {
        if (!root.wallet) {
            return
        }
        root.pendingDisplayName = root.wallet.displayName
        root.editingName = true
    }

    function cancelNameEdit() {
        root.pendingDisplayName = root.wallet ? root.wallet.displayName : ""
        root.editingName = false
    }

    function confirmNameEdit() {
        if (!root.wallet) {
            return
        }
        if (walletController.setWalletDisplayName(root.wallet.name, root.pendingDisplayName)) {
            root.editingName = false
        }
    }

    header: NavigationBar2 {
        leftItem: NavButton {
            objectName: "walletSettingsBackButton"
            iconSource: "image://images/caret-left"
            text: qsTr("Back")
            onClicked: root.back()
        }
        centerItem: Header {
            objectName: "walletSettingsHeader"
            headerBold: true
            headerSize: 18
            header: qsTr("Wallet settings")
        }
    }

    FileDialog {
        id: backupDialog
        fileMode: FileDialog.SaveFile
        currentFolder: "file://" + walletController.homePath()
        selectedFile: root.backupDefaultFileUrl()
        defaultSuffix: "bak"
        nameFilters: [qsTr("Wallet backup files (*.bak *.dat)"), qsTr("All files (*)")]
        onAccepted: {
            if (backupDialog.selectedFile.toString().length === 0) {
                return
            }
            const normalized = root.resolvedBackupPath(backupDialog.selectedFile.toString())
            if (!root.wallet.backupWallet(normalized)) {
                root.errorText = root.wallet.settingsError
            } else {
                root.errorText = ""
            }
        }
    }

    // Hidden automation hook so tests can inject a backup destination.
    TextField {
        id: backupAutomationPath
        objectName: "walletSettingsBackupPathField"
        visible: false
    }

    Connections {
        target: root.wallet
        function onSettingsErrorChanged() {
            root.errorText = root.wallet ? root.wallet.settingsError : ""
        }
        function onDisplayNameChanged() {
            if (!root.editingName) {
                root.pendingDisplayName = root.wallet ? root.wallet.displayName : ""
            }
        }
    }

    Connections {
        target: walletController
        function onSelectedWalletChanged() {
            root.editingName = false
            root.pendingDisplayName = root.wallet ? root.wallet.displayName : ""
        }
    }

    ColumnLayout {
        visible: !root.walletLoaded
        width: Math.min(parent.width, 450)
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        spacing: 24

        Header {
            objectName: "walletSettingsEmptyHeader"
            Layout.fillWidth: true
            header: qsTr("No wallet selected")
            headerBold: true
            headerSize: 28
            description: qsTr("Select a wallet to manage wallet-specific settings.")
        }

        OutlineButton {
            objectName: "walletSettingsSelectWalletButton"
            Layout.preferredWidth: 220
            Layout.alignment: Qt.AlignCenter
            text: qsTr("Select wallet")
            onClicked: root.selectWalletRequested()
        }
    }

    ColumnLayout {
        visible: root.walletLoaded
        width: Math.min(parent.width, 450)
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        spacing: 0

        EditableKeyValueRow {
            objectName: "walletSettingsNameRow"
            Layout.fillWidth: true
            Layout.topMargin: 12
            Layout.bottomMargin: 15
            keyObjectName: "walletSettingsNameKey"
            valueObjectName: "walletSettingsNameValue"
            editFieldObjectName: "walletSettingsNameEditField"
            editButtonObjectName: "walletSettingsNameEditButton"
            cancelButtonObjectName: "walletSettingsNameCancelButton"
            confirmButtonObjectName: "walletSettingsNameConfirmButton"
            label: qsTr("Name")
            displayValue: root.wallet ? root.wallet.displayName : ""
            editValue: root.pendingDisplayName
            editing: root.editingName
            onEditRequested: root.beginNameEdit()
            onCancelRequested: root.cancelNameEdit()
            onConfirmRequested: root.confirmNameEdit()
            onEditValueEdited: value => root.pendingDisplayName = value
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: Theme.color.neutral4 }

        KeyValueRow {
            keyWidth: 150
            Layout.fillWidth: true
            Layout.topMargin: 15
            Layout.bottomMargin: 15
            key: KeyText {
                objectName: "walletSettingsKeySchemeKey"
                text: qsTr("Key scheme")
            }
            value: ValueText {
                objectName: "walletSettingsKeySchemeValue"
                text: root.wallet ? root.wallet.keyScheme : ""
            }
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: Theme.color.neutral4 }

        KeyValueRow {
            keyWidth: 150
            Layout.fillWidth: true
            Layout.topMargin: 15
            Layout.bottomMargin: 15
            key: KeyText {
                objectName: "walletSettingsPrivateKeysKey"
                text: qsTr("Private keys")
            }
            value: ValueText {
                objectName: "walletSettingsPrivateKeysValue"
                text: root.wallet ? root.wallet.privateKeysStatus : ""
            }
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: Theme.color.neutral4 }

        KeyValueRow {
            keyWidth: 150
            Layout.fillWidth: true
            Layout.topMargin: 15
            Layout.bottomMargin: 15
            key: KeyText {
                objectName: "walletSettingsExternalSignerKey"
                text: qsTr("External signer")
            }
            value: ValueText {
                objectName: "walletSettingsExternalSignerValue"
                text: root.wallet ? root.wallet.externalSignerStatus : ""
            }
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: Theme.color.neutral4 }

        Setting {
            id: passwordSetting
            objectName: "walletSettingsPasswordRow"
            Layout.fillWidth: true
            header: root.wallet && root.wallet.isEncrypted ? qsTr("Update password") : qsTr("Set password")
            filledStateColor: Theme.color.neutral7
            hoverStateColor: Theme.color.orange
            activeStateColor: Theme.color.orange
            actionItem: CaretRightIcon {
                color: passwordSetting.stateColor
            }
            onClicked: root.passwordRequested()
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: Theme.color.neutral4 }

        Setting {
            id: backupSetting
            objectName: "walletSettingsBackupRow"
            Layout.fillWidth: true
            header: qsTr("Back up wallet")
            filledStateColor: Theme.color.neutral7
            hoverStateColor: Theme.color.orange
            activeStateColor: Theme.color.orange
            actionItem: CaretRightIcon {
                color: backupSetting.stateColor
            }
            onClicked: root.startBackup()
        }

        CoreText {
            objectName: "walletSettingsErrorText"
            Layout.fillWidth: true
            Layout.topMargin: 12
            visible: text.length > 0
            text: root.errorText
            color: Theme.color.red
            font.pixelSize: 15
            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WordWrap
        }

        CoreText {
            objectName: "walletSettingsDangerTitle"
            Layout.fillWidth: true
            Layout.topMargin: 36
            text: qsTr("Danger zone")
            color: Theme.color.red
            bold: true
            font.pixelSize: 24
            horizontalAlignment: Text.AlignLeft
        }

        CoreText {
            objectName: "walletSettingsDangerDescription"
            Layout.fillWidth: true
            Layout.topMargin: 8
            text: qsTr("Remove this wallet permanently from this device.")
            color: Theme.color.neutral8
            font.pixelSize: 15
            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WordWrap
        }

        Rectangle { Layout.fillWidth: true; Layout.topMargin: 12; height: 1; color: Theme.color.neutral4 }

        Setting {
            id: deleteSetting
            objectName: "walletSettingsDeleteRow"
            Layout.fillWidth: true
            header: qsTr("Delete wallet")
            filledStateColor: Theme.color.neutral7
            hoverStateColor: Theme.color.red
            activeStateColor: Theme.color.red
            actionItem: CaretRightIcon {
                color: deleteSetting.stateColor
            }
            onClicked: root.deleteWalletRequested()
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
