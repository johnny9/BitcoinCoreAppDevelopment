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

    x: Math.round((parent.width - width) / 2)
    y: Math.round((parent.height - height) / 2)
    width: 100
    height: 100

    Rectangle {
        color: "lightskyblue"
        implicitWidth: 400
        implicitHeight: 100
        Text {
            text: "Hello blue sky!"
            color: "navy"
            anchors.centerIn: parent
        }
    }
}