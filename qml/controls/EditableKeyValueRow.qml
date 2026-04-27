// Copyright (c) 2026 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.bitcoincore.qt 1.0

Item {
    id: root

    property string label: ""
    property string displayValue: ""
    property string editValue: ""
    property bool editing: false
    property bool confirmEnabled: root.editValue.trim().length > 0
    property int keyWidth: 150
    property int fieldWidth: 152
    property string keyObjectName: ""
    property string valueObjectName: ""
    property string editFieldObjectName: ""
    property string editButtonObjectName: ""
    property string cancelButtonObjectName: ""
    property string confirmButtonObjectName: ""

    signal editRequested()
    signal cancelRequested()
    signal confirmRequested()
    signal editValueEdited(string value)

    implicitHeight: 36

    RowLayout {
        anchors.fill: parent
        spacing: 10

        CoreText {
            objectName: root.keyObjectName
            Layout.preferredWidth: root.keyWidth
            text: root.label
            color: Theme.color.neutral7
            font.pixelSize: 18
            font.styleName: "Regular"
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.NoWrap
        }

        Item {
            Layout.fillWidth: true
        }

        Loader {
            Layout.alignment: Qt.AlignVCenter
            active: root.editing
            visible: active
            sourceComponent: RowLayout {
                spacing: 8

                TextField {
                    objectName: root.editFieldObjectName
                    Layout.preferredWidth: root.fieldWidth
                    implicitHeight: 36
                    text: root.editValue
                    color: Theme.color.neutral9
                    font.family: "Inter"
                    font.styleName: "Regular"
                    font.pixelSize: 18
                    leftPadding: 12
                    rightPadding: 12
                    selectByMouse: true
                    background: Rectangle {
                        border.color: Theme.color.neutral5
                        border.width: 1
                        color: "transparent"
                        radius: 5
                    }
                    onTextChanged: root.editValueEdited(text)
                    Component.onCompleted: forceActiveFocus()
                    Keys.onReturnPressed: root.confirmRequested()
                    Keys.onEnterPressed: root.confirmRequested()
                    Keys.onEscapePressed: root.cancelRequested()
                }

                ActionButton {
                    objectName: root.cancelButtonObjectName
                    tintIcon: false
                    imageSource: "qrc:/icons/circle-red-cross"
                    imageSize: 30
                    onClicked: root.cancelRequested()
                }

                ActionButton {
                    objectName: root.confirmButtonObjectName
                    enabled: root.confirmEnabled
                    tintIcon: false
                    imageSource: "qrc:/icons/circle-green-check"
                    imageSize: 30
                    onClicked: root.confirmRequested()
                }
            }
        }

        Loader {
            Layout.alignment: Qt.AlignVCenter
            active: !root.editing
            visible: active
            sourceComponent: RowLayout {
                spacing: 6

                CoreText {
                    objectName: root.valueObjectName
                    text: root.displayValue
                    color: Theme.color.neutral9
                    font.pixelSize: 18
                    font.styleName: "Regular"
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    wrapMode: Text.WordWrap
                }

                ActionButton {
                    objectName: root.editButtonObjectName
                    iconSource: "image://images/edit"
                    iconColor: Theme.color.neutral9
                    hoverColor: Theme.color.orange
                    activeColor: Theme.color.orange
                    onClicked: root.editRequested()
                }
            }
        }
    }

    component ActionButton: Button {
        id: actionButton

        property bool tintIcon: true
        property url iconSource: ""
        property color iconColor: Theme.color.neutral9
        property color hoverColor: Theme.color.orange
        property color activeColor: hoverColor
        property int iconSize: 20
        property url imageSource: ""
        property int imageSize: 20

        implicitWidth: 30
        implicitHeight: 30
        hoverEnabled: AppMode.isDesktop
        padding: 0

        HoverHandler {
            enabled: actionButton.enabled && AppMode.isDesktop
            cursorShape: Qt.PointingHandCursor
        }

        background: Rectangle {
            anchors.fill: parent
            radius: 5
            color: !actionButton.enabled
                ? "transparent"
                : actionButton.pressed
                    ? Theme.color.neutral3
                    : actionButton.hovered
                        ? Theme.color.neutral2
                        : "transparent"

            Behavior on color {
                ColorAnimation { duration: 150 }
            }
        }

        contentItem: Item {
            Icon {
                visible: actionButton.tintIcon
                anchors.centerIn: parent
                size: actionButton.iconSize
                source: actionButton.iconSource
                color: !actionButton.enabled
                    ? Theme.color.neutral4
                    : actionButton.pressed
                        ? actionButton.activeColor
                        : actionButton.hovered
                            ? actionButton.hoverColor
                            : actionButton.iconColor

                Behavior on color {
                    ColorAnimation { duration: 150 }
                }
            }

            Image {
                visible: !actionButton.tintIcon
                anchors.centerIn: parent
                width: actionButton.imageSize
                height: actionButton.imageSize
                source: actionButton.imageSource
                sourceSize.width: actionButton.imageSize
                sourceSize.height: actionButton.imageSize
                fillMode: Image.PreserveAspectFit
                opacity: actionButton.enabled ? 1 : 0.45

                Behavior on opacity {
                    NumberAnimation { duration: 150 }
                }
            }
        }
    }
}
