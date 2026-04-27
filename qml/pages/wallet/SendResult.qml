// Copyright (c) 2024 The Bitcoin Core developers
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
    objectName: "sendResultPopup"

    property bool opened: visible
    property string descriptionText: qsTr("Based on your selected fee, it should be confirmed within the next 10 minutes.")
    property string actionText: qsTr("Done")

    background: Rectangle {
        color: Theme.color.background
    }

    enum ResultType {
        Regular,
        SpeedUp
        /*, Cancel */
    }

    property int resultType: SendResult.ResultType.Regular

    signal done()
    signal viewNewTransaction()

    ColumnLayout {
        id: columnLayout
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -60
        width: Math.min(parent.width - 80, 450)
        spacing: 20

        Item {
            width: 60
            height: 60
            Layout.alignment: Qt.AlignHCenter
            Rectangle {
                anchors.fill: parent
                radius: 30
                color: Theme.color.green
                opacity: 0.2
            }
            Icon {
                anchors.centerIn: parent
                source: "qrc:/icons/check"
                color: Theme.color.green
                size: 30
            }
        }

        CoreText {
            Layout.alignment: Qt.AlignHCenter
            text: root.resultType === SendResult.ResultType.SpeedUp
                ? qsTr("Transaction updated")
                : qsTr("Transaction sent")
            font.pixelSize: 28
            bold: true
        }

        CoreText {
            Layout.alignment: Qt.AlignHCenter
            Layout.maximumWidth: 350
            Layout.topMargin: 10
            Layout.bottomMargin: 20
            color: Theme.color.neutral7
            text: root.descriptionText
            font.pixelSize: 18
        }

        RowLayout {
            Layout.alignment: Qt.AlignCenter
            spacing: 15

            OutlineButton {
                text: root.resultType === SendResult.ResultType.SpeedUp
                    ? qsTr("View new transaction")
                    : qsTr("View transaction")
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                Layout.minimumWidth: 150
                onClicked: root.viewNewTransaction()
            }

            ContinueButton {
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                Layout.minimumWidth: 150
                text: root.actionText
                onClicked: root.done()
            }
        }
    }
}
