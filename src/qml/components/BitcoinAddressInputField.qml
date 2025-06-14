// Copyright (c) 2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.bitcoincore.qt 1.0

import "../controls"

ColumnLayout {
    id: root

    property var address
    property string errorText: ""
    property string labelText: qsTr("Send to")
    property bool enabled: true

    signal editingFinished()

    Layout.fillWidth: true
    spacing: 4

    LabeledTextInput {
        id: addressInput
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignTop
        enabled: root.enabled
        labelText: root.labelText
        placeholderText: qsTr("Enter address...")
        text: root.address.formattedAddress

        onTextEdited: {
            if (root.address) {
                cursorPosition = root.address.setAddress(text, cursorPosition)
            }
        }


        onEditingFinished: {
            if (root.address) {
                root.address.setAddress(text, cursorPosition)
                root.editingFinished()
            }
        }


        onActiveFocusChanged: {
            if (!activeFocus && root.address) {
                root.address.setAddress(text)
            }
        }
    }


    RowLayout {
        id: addressIssue
        Layout.fillWidth: true
        visible: root.errorText.length > 0
        spacing: 8
        Layout.topMargin: 4

        Icon {
            source: "image://images/alert-filled"
            size: 22
            color: Theme.color.red
        }

        CoreText {
            text: root.errorText
            font.pixelSize: 15
            color: Theme.color.red
            horizontalAlignment: Text.AlignLeft
            Layout.fillWidth: true
        }
    }
}
