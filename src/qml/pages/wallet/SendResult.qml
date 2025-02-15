// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.2
import org.bitcoincore.qt 1.0

import "../../controls"
import "../../components"

Popup {
    modal: true
    anchors.centerIn: parent

    Rectangle {
        color: Theme.color.neutral0
        border.color: Theme.color.neutral4
        border.width: 1
        radius: 5
        implicitWidth: 400
        implicitHeight: 100
        ColumnLayout {
            CoreText {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Transaction sent")
            }
        }
    }
}