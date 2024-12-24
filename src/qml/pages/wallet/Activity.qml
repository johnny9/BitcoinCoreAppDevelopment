// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.bitcoincore.qt 1.0

import "../../controls"
import "../../components"

PageStack {
    id: stackView

    initialItem: Page {
        id: root
        background: null

        CoreText {
            id: title
            text: qsTr("Activity")
            anchors.left: listView.left
            anchors.top: parent.top
            anchors.topMargin: 20
            font.pixelSize: 21
            bold: true
        }

        ListView {
            id: listView
            clip: true
            width: Math.min(parent.width - 40, 600)
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: title.bottom
            anchors.bottom: parent.bottom

            model: activityListModel
            delegate: ItemDelegate {
                id: delegate
                required property string label;
                required property string date;
                required property string amount;
                required property int type;
                required property int status;

                width: listView.width
                height: 51

                background: Item {
                    Separator {
                        anchors.bottom: parent.bottom
                        width: parent.width
                    }
                }

                contentItem: Item {
                    Icon {
                        id: directionIcon
                        anchors.left: parent.left
                        anchors.margins: 6
                        anchors.verticalCenter: parent.verticalCenter
                        source: {
                            if (delegate.type == Transaction.RecvWithAddress || delegate.type == Transaction.RecvFromOther) {
                                "qrc:/icons/triangle-down"
                            } else {
                                "qrc:/icons/triangle-up"
                            }
                        }
                        color: {
                            if (delegate.status == Transaction.Confirmed) {
                                if (delegate.type == Transaction.RecvWithAddress ||
                                    delegate.type == Transaction.RecvFromOther) {
                                    Theme.color.green
                                } else {
                                    Theme.color.orange
                                }
                            } else {
                                Theme.color.blue
                            }
                        }
                        size: 12
                    }
                    CoreText {
                        id: label
                        anchors.left: directionIcon.right
                        anchors.right: date.left
                        anchors.margins: 6
                        anchors.verticalCenter: parent.verticalCenter
                        text: delegate.label
                        font.pixelSize: 15
                        horizontalAlignment: Text.AlignLeft
                    }

                    CoreText {
                        id: date
                        anchors.right: amount.left
                        anchors.margins: 6
                        anchors.verticalCenter: parent.verticalCenter
                        text: delegate.date
                        font.pixelSize: 15
                        horizontalAlignment: Text.AlignRight
                    }

                    CoreText {
                        id: amount
                        width: 125
                        anchors.right: parent.right
                        anchors.margins: 6
                        anchors.verticalCenter: parent.verticalCenter
                        text: delegate.amount
                        font.pixelSize: 15
                        horizontalAlignment: Text.AlignLeft
                        color: delegate.direction == "receiving" ? Theme.color.green : Theme.color.neutral9
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: stackView.push(detailsPage)
                        hoverEnabled: true
                        onEntered: label.color = Theme.color.orange
                        onExited: label.color = Theme.color.neutral9
                        cursorShape: Qt.PointingHandCursor
                    }

                    Component {
                        id: detailsPage
                        ActivityDetails {
                            amount: delegate.amount
                        }
                    }
                }
            }
        }
    }
}
