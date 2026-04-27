// Copyright (c) 2026 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../controls"

Item {
    id: root

    property string labelText: qsTr("Amount")
    property string amountText: ""
    property string unitText: ""
    property string text: unitText.length > 0 ? amountText + " " + unitText : amountText
    property int labelWidth: 110
    property int labelPixelSize: 18
    property color labelColor: Theme.color.neutral9

    Layout.fillWidth: true
    implicitHeight: Math.max(label.implicitHeight, amountValue.implicitHeight, unitLabel.implicitHeight)

    CoreText {
        id: label
        width: root.labelWidth
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        horizontalAlignment: Text.AlignLeft
        wrap: false
        text: root.labelText
        font.pixelSize: root.labelPixelSize
        color: root.labelColor
    }

    CoreText {
        id: amountValue
        anchors.left: label.right
        anchors.right: unitLabel.left
        anchors.rightMargin: unitLabel.visible ? 12 : 0
        anchors.verticalCenter: parent.verticalCenter
        horizontalAlignment: Text.AlignRight
        wrap: false
        elide: Text.ElideRight
        text: root.amountText
        font.pixelSize: 18
        color: Theme.color.neutral9
    }

    CoreText {
        id: unitLabel
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        visible: root.unitText.length > 0
        text: root.unitText
        font.pixelSize: 18
        color: Theme.color.neutral7
    }
}
