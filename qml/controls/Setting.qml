// Copyright (c) 2022 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.bitcoincore.qt 1.0

AbstractButton {
    id: root
    property string header
    property alias actionItem: action_loader.sourceComponent
    property alias loadedItem: action_loader.item
    property string description
    property string errorText: ""
    property bool showErrorText: false
    property color stateColor
    property color stateDescriptionColor
    property bool disabled: false
    hoverEnabled: AppMode.isDesktop
    state: "FILLED"
    onDisabledChanged: state = disabled ? "DISABLED" : "FILLED"

    states: [
        State {
            name: "FILLED"
            PropertyChanges {
                target: root
                enabled: true
                stateColor: Theme.color.neutral9
                stateDescriptionColor: Theme.color.neutral8
            }
        },
        State {
            name: "HOVER"
            PropertyChanges { target: root; stateColor: Theme.color.orangeLight1; stateDescriptionColor: Theme.color.neutral8 }
        },
        State {
            name: "ACTIVE"
            PropertyChanges { target: root; stateColor: Theme.color.orange; stateDescriptionColor: Theme.color.neutral8 }
        },
        State {
            name: "DISABLED"
            PropertyChanges {
                target: root
                enabled: false
                stateColor: Theme.dark ? Theme.color.neutral4 : Theme.color.neutral6
                stateDescriptionColor: Theme.dark ? Theme.color.neutral4 : Theme.color.neutral6
            }
        }
    ]

    background: FocusBorder {
        visible: root.visualFocus
        topMargin: -4
        bottomMargin: -4
        leftMargin: -6
        rightMargin: -6
    }

    MouseArea {
        id: mouseArea
        anchors.fill: root
        hoverEnabled: AppMode.isDesktop
        onEntered: {
            if (root.state !== "DISABLED") root.state = "HOVER"
        }
        onExited: {
            if (root.state !== "DISABLED") root.state = "FILLED"
        }
        onPressed: {
            if (!root.disabled) root.state = "ACTIVE"
        }
        onReleased: {
            if (!root.disabled) {
                if (mouseArea.containsMouse) {
                    root.state = "HOVER"
                    root.clicked()
                } else {
                    root.state = "FILLED"
                }
            }
        }
    }

    contentItem: RowLayout {
        Header {
            Layout.topMargin: 14
            Layout.bottomMargin: 14
            Layout.fillWidth: true
            center: false
            header: root.header
            headerSize: 18
            headerColor: root.stateColor
            descriptionColor: root.stateDescriptionColor
            description: root.description
            descriptionSize: 15
            descriptionMargin: 0
            subtext: root.showErrorText ? root.errorText : ""
            subtextColor: Theme.color.blue
        }
        Loader {
            id: action_loader
            active: true
            visible: active
            sourceComponent: root.actionItem
        }
    }
}
