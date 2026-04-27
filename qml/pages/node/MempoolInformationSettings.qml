// Copyright (c) 2026 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15
import QtQuick.Controls 2.15

import "../../controls"
import "../../components"

InformationPage {
    id: root
    objectName: "mempoolInformationSettingsPage"

    navLeftDetail: NavButton {
        objectName: "mempoolInformationBackButton"
        iconSource: "image://images/caret-left"
        text: qsTr("Back")
        onClicked: root.back()
    }

    navMiddleDetail: Header {
        objectName: "mempoolInformationHeader"
        headerBold: true
        headerSize: 18
        header: qsTr("Mempool information")
    }

    bannerActive: false
    bold: true
    showHeader: false
    headerText: ""
    headerMargin: 0
    description: ""
    descriptionMargin: 0
    detailActive: true
    detailTopMargin: 0
    detailMaximumWidth: 450
    detailItem: MempoolInformationRows {
        id: mempoolInformationRows
    }

    Component.onCompleted: nodeModel.refreshMempoolInfo()
    onVisibleChanged: {
        if (visible) {
            nodeModel.refreshMempoolInfo()
        }
    }

    Timer {
        interval: 3000
        repeat: true
        running: root.visible
        onTriggered: nodeModel.refreshMempoolInfo()
    }
}
