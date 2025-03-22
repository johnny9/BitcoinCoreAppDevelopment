// Copyright (c) 2025 The Bitcoin Core developers
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

    property WalletQmlModel wallet: walletController.selectedWallet

    signal done()

    header: NavigationBar2 {
        centerItem: Header {
            headerBold: true
            headerSize: 18
            header: qsTr("Coin Selection")
        }
        rightItem: NavButton {
            text: qsTr("Done")
            onClicked: root.done()
        }
    }

    background: Rectangle {
        color: Theme.color.neutral0
    }

    ListView {
        id: listView
        clip: true
        width: Math.min(parent.width - 40, 450)
        height: parent.height
        anchors.horizontalCenter: parent.horizontalCenter
        model: root.wallet.coinsListModel
        spacing: 15

        header: ColumnLayout {
            spacing: 20
            width: parent.width

            Flickable {
                id: sortSelection
                Layout.fillWidth: true
                Layout.bottomMargin: 30
                Layout.alignment: Qt.AlignHCenter
                height: toggleButtons.height
                contentWidth: toggleButtons.width
                boundsMovement: width == toggleButtons.width ?
                    Flickable.StopAtBound : Flickable.FollowBoundsBehavior
                RowLayout {
                    id: toggleButtons
                    spacing: 10
                    ToggleButton {
                        text: qsTr("date")
                        autoExclusive: true
                    }
                    ToggleButton {
                        text: qsTr("amount")
                        autoExclusive: true
                    }
                    ToggleButton {
                        text: qsTr("label")
                        autoExclusive: true
                    }
                }
            }
        }

        delegate: ItemDelegate {
            id: delegate
            required property string address;
            required property string amount;
            required property string label;
            required property bool locked;

            required property int index;

            readonly property color stateColor: {
                if (delegate.down) {
                    return Theme.color.orange
                } else if (delegate.hovered) {
                    return Theme.color.orangeLight1
                }
                return Theme.color.neutral9
            }

            leftPadding: 0
            rightPadding: 0
            topPadding: 0
            bottomPadding: 14
            width: listView.width

            background: Item {
                Separator {
                    anchors.bottom: parent.bottom
                    width: parent.width
                }
            }

            contentItem: RowLayout {
                width: parent.width
                CoreCheckBox {
                    id: checkBox
                    Layout.minimumWidth: 20
                    checked: locked
                    MouseArea {
                        anchors.fill: parent
                        enabled: false
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                    }

                    onToggled: {
                        // TODO: Figure out how to error check this
                        listView.model.toggleCoinLock(index)
                    }
                }
                CoreText {
                    text: amount
                    font.pixelSize: 18
                }
                CoreText {
                    Layout.fillWidth: true
                    text: label != "" ? label : address
                    font.pixelSize: 18
                    elide: Text.ElideMiddle
                    wrapMode: Text.NoWrap
                }
            }
        }
    }
}