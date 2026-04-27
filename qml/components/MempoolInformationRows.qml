// Copyright (c) 2026 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick 2.15 as QtQuickBase

import "../controls"

ColumnLayout {
    id: root
    spacing: 4

    function formatMegabytes(valueMb) {
        const rounded = Math.round(valueMb)
        const decimals = Math.abs(valueMb - rounded) < 0.005 ? 0 : 2
        return Number(valueMb).toLocaleString(Qt.locale(), 'f', decimals) + " MB"
    }

    Setting {
        objectName: "mempoolTransactionsRow"
        Layout.fillWidth: true
        state: "DISABLED"
        disabledStateColor: Theme.color.neutral9
        header: qsTr("Transactions")
        actionItem: CoreText {
            text: Number(nodeModel.mempoolTransactionCount).toLocaleString(Qt.locale(), 'f', 0)
            color: Theme.color.neutral7
            font.pixelSize: 15
            fontStyleName: "Regular"
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.NoWrap
        }
    }

    Separator { Layout.fillWidth: true }

    Setting {
        objectName: "mempoolMemoryUsedRow"
        Layout.fillWidth: true
        state: "DISABLED"
        disabledStateColor: Theme.color.neutral9
        header: qsTr("Memory used")
        actionItem: CoreText {
            text: qsTr("%1 / %2")
                .arg(root.formatMegabytes(nodeModel.mempoolUsageMB))
                .arg(root.formatMegabytes(nodeModel.mempoolMaxUsageMB))
            color: Theme.color.neutral7
            font.pixelSize: 15
            fontStyleName: "Regular"
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.NoWrap
        }
    }

    Separator { Layout.fillWidth: true }

    Setting {
        id: mempoolLimitSetting
        objectName: "mempoolSizeLimitRow"
        Layout.fillWidth: true
        header: qsTr("Mempool size limit")
        description: qsTr("Applies after restart")
        descriptionColor: Theme.color.neutral7
        errorText: qsTr("This is not a valid mempool size. Please choose a value between %1 and %2 MB.")
            .arg(optionsModel.minMaxMempoolSizeMB)
            .arg(optionsModel.maxMaxMempoolSizeMB)
        showErrorText: false
        actionItem: ValueInput {
            parentState: mempoolLimitSetting.state
            description: optionsModel.maxMempoolSizeMB
            descriptionSize: 15
            validator: QtQuickBase.IntValidator {
                bottom: optionsModel.minMaxMempoolSizeMB
                top: optionsModel.maxMaxMempoolSizeMB
            }
            onEditingFinished: {
                if (checkValidity(optionsModel.minMaxMempoolSizeMB, optionsModel.maxMaxMempoolSizeMB, parseInt(text))) {
                    optionsModel.maxMempoolSizeMB = parseInt(text)
                    mempoolLimitSetting.forceActiveFocus()
                    mempoolLimitSetting.showErrorText = false
                } else {
                    mempoolLimitSetting.showErrorText = true
                }
            }
        }
        onClicked: {
            loadedItem.filled = true
            loadedItem.forceActiveFocus()
        }
    }
}
