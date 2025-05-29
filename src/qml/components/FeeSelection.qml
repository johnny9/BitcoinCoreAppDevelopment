// Copyright (c) 2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../controls"
import "../components"

Item {
    id: root

    property int selectedIndex: 1
    property string selectedLabel: feeModel.get(selectedIndex).feeLabel
    signal feeChanged(int index)

    width: parent ? parent.width : 300
    height: 40

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onClicked: feePopup.open()
    }

    CoreText {
        id: label
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        text: qsTr("Fee")
        font.pixelSize: 15
    }

    CoreText {
        id: value
        anchors.right: caret.left
        anchors.verticalCenter: parent.verticalCenter
        text: root.selectedLabel
        font.pixelSize: 15
    }

    Icon {
        id: caret
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        source: "image://images/caret-right"
        size: 30
        color: enabled ? Theme.color.accent : Theme.color.neutral2
    }

    Popup {
        id: feePopup
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        background: Rectangle {
            color: Theme.color.neutral1
            radius: 6
            border.color: Theme.color.neutral3
        }

        width: 260
        height: Math.min(feeModel.count * 40 + 20, 300)
        x: parent.width - width
        y: parent.height

        contentItem: ListView {
            id: feeList
            model: feeModel
            interactive: false
            width: 260
            height: contentHeight
            delegate: Item {
                required property string feeLabel
                required property int index

                width: ListView.view.width
                height: 40

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 10

                    CoreText { text: feeLabel; font.pixelSize: 15 }

                    Item { Layout.fillWidth: true }

                    Icon {
                        visible: index === root.selectedIndex
                        source: "image://images/check"
                        color: Theme.color.neutral9
                        size: 20
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        root.selectedIndex = index
                        root.selectedLabel = feeLabel
                        root.feeChanged(index, sats)
                        feePopup.close()
                    }
                }

                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: 1
                    color: Theme.color.neutral3
                    visible: index < feeModel.count - 1
                }
            }
        }
    }

    ListModel {
        id: feeModel
        ListElement { feeLabel: qsTr("High (~10 mins)") }
        ListElement { feeLabel: qsTr("Default (~20 mins)") }
        ListElement { feeLabel: qsTr("Low (~60 mins)") }
    }
}
