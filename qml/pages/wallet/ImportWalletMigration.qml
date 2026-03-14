// Copyright (c) 2026 The Bitcoin Core developers
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
    objectName: "importWalletMigration"
    required property string walletPath
    signal back
    signal cancel
    signal next

    property string phase: "intro"
    readonly property bool introPhase: phase === "intro"
    readonly property bool updatingPhase: phase === "updating"
    readonly property bool successPhase: phase === "success"
    readonly property bool failedPhase: phase === "failed"

    function startMigration() {
        phase = "updating"
        walletController.migrateWallet(walletPath)
    }

    Component.onCompleted: {
        walletController.clearWalletMigrationStatus()
        phase = "intro"
    }

    background: null

    header: NavigationBar2 {
        leftItem: NavButton {
            visible: !root.updatingPhase
            enabled: visible
            iconSource: "image://images/caret-left"
            text: qsTr("Back")
            onClicked: {
                walletController.clearWalletMigrationStatus()
                root.back()
            }
        }
        rightItem: NavButton {
            visible: root.introPhase || root.failedPhase
            enabled: visible
            text: qsTr("Cancel")
            onClicked: {
                walletController.clearWalletMigrationStatus()
                root.cancel()
            }
        }
    }

    Connections {
        target: walletController
        function onWalletMigrationSucceeded() {
            root.phase = "success"
        }
        function onWalletMigrationFailed() {
            root.phase = "failed"
        }
    }

    ColumnLayout {
        width: Math.min(parent.width, 450)
        anchors.horizontalCenter: parent.horizontalCenter

        Image {
            Layout.alignment: Qt.AlignCenter
            Layout.topMargin: 20
            source: {
                if (root.successPhase) {
                    return "image://images/circle-green-check"
                }
                if (root.failedPhase) {
                    return "image://images/circle-red-cross"
                }
                return "image://images/pending"
            }
            sourceSize.width: 60
            sourceSize.height: 60
        }

        Header {
            Layout.topMargin: 18
            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            header: {
                if (root.successPhase) {
                    return qsTr("Update completed")
                }
                if (root.failedPhase) {
                    return qsTr("Update failed")
                }
                if (root.updatingPhase) {
                    return qsTr("Updating wallet")
                }
                return qsTr("Wallet update required")
            }
            headerBold: true
            description: {
                if (root.successPhase) {
                    return qsTr("You can use your wallet as usual.")
                }
                if (root.failedPhase) {
                    return walletController.walletMigrationError.length > 0
                        ? walletController.walletMigrationError
                        : qsTr("The wallet could not be updated.")
                }
                if (root.updatingPhase) {
                    return qsTr("Updating your wallet now. This may take a moment.")
                }
                return qsTr("This wallet uses an outdated file format and needs to be updated. This does not impact security or any previously made transactions.")
            }
        }

        ContinueButton {
            id: actionButton
            objectName: "walletMigrationActionButton"
            Layout.preferredWidth: Math.min(335, parent.width - 2 * Layout.leftMargin)
            Layout.topMargin: 30
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            Layout.alignment: Qt.AlignCenter
            enabled: !root.updatingPhase
            text: {
                if (root.successPhase) {
                    return qsTr("Next")
                }
                if (root.failedPhase) {
                    return qsTr("Back to overview")
                }
                if (root.updatingPhase) {
                    return qsTr("Updating wallet...")
                }
                return qsTr("Update wallet")
            }
            onClicked: {
                if (root.successPhase) {
                    walletController.clearWalletMigrationStatus()
                    root.next()
                    return
                }
                if (root.failedPhase) {
                    walletController.clearWalletMigrationStatus()
                    root.cancel()
                    return
                }
                root.startMigration()
            }
        }
    }
}
