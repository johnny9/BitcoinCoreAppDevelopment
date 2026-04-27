// Copyright (c) 2025-2026 The Bitcoin Core developers
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
    property string inputObjectName: ""
    property bool enabled: true

    signal textChanged()
    signal editingFinished()

    Layout.fillWidth: true
    spacing: 4

    Item {
        id: inputRow
        Layout.fillWidth: true
        implicitHeight: Math.max(label.implicitHeight + 6, addressInput.height)

        CoreText {
            id: label
            width: 110
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.topMargin: 6
            horizontalAlignment: Text.AlignLeft
            text: root.labelText
            font.pixelSize: 18
        }

        TextArea {
            id: addressInput
            objectName: root.inputObjectName
            anchors.left: label.right
            anchors.right: parent.right
            anchors.top: parent.top
            enabled: root.enabled
            placeholderText: qsTr("Enter address...")
            text: root.address ? root.address.formattedAddress : ""
            wrapMode: Text.WrapAnywhere
            leftPadding: 0
            topPadding: 0
            rightPadding: 0
            bottomPadding: 0
            height: Math.max(contentHeight, 32)
            font.family: "Inter"
            font.styleName: "Regular"
            font.pixelSize: 18
            color: Theme.color.neutral9
            placeholderTextColor: enabled ? Theme.color.neutral7 : Theme.color.neutral4
            background: Item {}
            selectByMouse: true

            onTextChanged: {
                if (root.address) {
                    cursorPosition = root.address.setAddress(text, cursorPosition)
                }
                root.textChanged()
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
