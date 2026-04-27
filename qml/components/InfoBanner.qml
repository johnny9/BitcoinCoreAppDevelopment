// Copyright (c) 2026 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.bitcoincore.qt 1.0

import "../controls"

Rectangle {
    id: root

    enum Layout {
        Horizontal,
        Vertical
    }

    property url iconSource: ""
    property string title: ""
    property string message: ""
    property string primaryButtonText: ""
    property string dismissButtonText: ""
    property int bannerLayout: InfoBanner.Layout.Horizontal

    signal primaryClicked()
    signal dismissClicked()

    radius: 10
    color: Qt.rgba(Theme.color.blue.r, Theme.color.blue.g, Theme.color.blue.b, 0.3)
    implicitHeight: contentLoader.item ? contentLoader.item.height + 60 : 60

    Loader {
        id: contentLoader
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.margins: 30
        sourceComponent: root.bannerLayout === InfoBanner.Layout.Vertical
            ? verticalContent : horizontalContent
    }

    Component {
        id: horizontalContent
        RowLayout {
            spacing: 15

            Icon {
                visible: root.iconSource != ""
                source: root.iconSource
                color: Theme.color.neutral7
                size: 24
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                CoreText {
                    visible: root.title !== ""
                    text: root.title
                    font.pixelSize: 15
                    bold: true
                    color: Theme.color.neutral9
                    horizontalAlignment: Text.AlignLeft
                    Layout.fillWidth: true
                }

                CoreText {
                    visible: root.message !== ""
                    text: root.message
                    font.pixelSize: 13
                    color: Theme.color.neutral7
                    horizontalAlignment: Text.AlignLeft
                    Layout.fillWidth: true
                    wrap: true
                }
            }

            OutlineButton {
                objectName: root.objectName !== "" ? root.objectName + "DismissButton" : ""
                visible: root.dismissButtonText !== ""
                text: root.dismissButtonText
                onClicked: root.dismissClicked()
            }

            ContinueButton {
                objectName: root.objectName !== "" ? root.objectName + "PrimaryButton" : ""
                visible: root.primaryButtonText !== ""
                text: root.primaryButtonText
                onClicked: root.primaryClicked()
            }
        }
    }

    Component {
        id: verticalContent
        ColumnLayout {
            spacing: 15

            Icon {
                visible: root.iconSource != ""
                source: root.iconSource
                color: Theme.color.neutral7
                size: 24
                Layout.alignment: Qt.AlignHCenter
            }

            CoreText {
                visible: root.title !== ""
                text: root.title
                font.pixelSize: 15
                bold: true
                color: Theme.color.neutral9
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
            }

            CoreText {
                visible: root.message !== ""
                text: root.message
                font.pixelSize: 13
                color: Theme.color.neutral7
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
                Layout.bottomMargin: 10
                wrap: true
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 15

                OutlineButton {
                    objectName: root.objectName !== "" ? root.objectName + "DismissButton" : ""
                    visible: root.dismissButtonText !== ""
                    text: root.dismissButtonText
                    Layout.maximumWidth: 200
                    onClicked: root.dismissClicked()
                }

                ContinueButton {
                    objectName: root.objectName !== "" ? root.objectName + "PrimaryButton" : ""
                    visible: root.primaryButtonText !== ""
                    text: root.primaryButtonText
                    Layout.maximumWidth: 200
                    leftPadding: 30
                    rightPadding: 30
                    onClicked: root.primaryClicked()
                }
            }
        }
    }
}
