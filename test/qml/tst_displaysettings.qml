// Copyright (c) 2026 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtTest 1.2
import "../../qml/controls"

TestCase {
    name: "DisplaySettings"
    when: windowShown
    width: 600
    height: 800

    // Minimal component exercising the BTC/SAT OptionButton binding logic.
    // Uses OptionButton directly (no NavButton / org.bitcoincore.qt dependency)
    // to test the optionsModel.displayUnit binding in isolation.
    // ButtonGroup is intentionally omitted: the declarative 'checked:' bindings
    // already model mutual exclusion through optionsModel, and ButtonGroup's
    // managed-checked behavior conflicts with declarative bindings in tests.
    Component {
        id: displayUnitButtons
        Column {
            OptionButton {
                objectName: "displayUnitBTC"
                text: "BTC"
                checked: optionsModel.displayUnit === 0
                onClicked: optionsModel.displayUnit = 0
            }
            OptionButton {
                objectName: "displayUnitSAT"
                text: "sat"
                checked: optionsModel.displayUnit === 1
                onClicked: optionsModel.displayUnit = 1
            }
        }
    }

    function test_displayUnit_BTC_button_checked_by_default() {
        optionsModel.displayUnit = 0
        const obj = createTemporaryObject(displayUnitButtons, this)
        verify(obj !== null)

        const btcBtn = findChild(obj, "displayUnitBTC")
        verify(btcBtn !== null)
        compare(btcBtn.checked, true)

        const satBtn = findChild(obj, "displayUnitSAT")
        verify(satBtn !== null)
        compare(satBtn.checked, false)
    }

    function test_displayUnit_SAT_button_updates_on_model_change() {
        optionsModel.displayUnit = 1
        const obj = createTemporaryObject(displayUnitButtons, this)
        verify(obj !== null)

        const satBtn = findChild(obj, "displayUnitSAT")
        verify(satBtn !== null)
        compare(satBtn.checked, true)

        const btcBtn = findChild(obj, "displayUnitBTC")
        verify(btcBtn !== null)
        compare(btcBtn.checked, false)

        // Reset
        optionsModel.displayUnit = 0
    }

    function test_displayUnit_clicking_SAT_updates_model() {
        optionsModel.displayUnit = 0
        const obj = createTemporaryObject(displayUnitButtons, this)
        verify(obj !== null)

        const satBtn = findChild(obj, "displayUnitSAT")
        verify(satBtn !== null)

        // Invoke the onClicked handler directly to simulate a user press.
        satBtn.clicked()
        compare(optionsModel.displayUnit, 1)

        // Reset
        optionsModel.displayUnit = 0
    }

    function test_displayUnit_clicking_BTC_after_SAT_resets_model() {
        optionsModel.displayUnit = 1
        const obj = createTemporaryObject(displayUnitButtons, this)
        verify(obj !== null)

        const btcBtn = findChild(obj, "displayUnitBTC")
        verify(btcBtn !== null)
        compare(btcBtn.checked, false)

        btcBtn.clicked()
        compare(optionsModel.displayUnit, 0)
    }

    // Mirrors the balance prefix expression in WalletBadge.qml.
    Component {
        id: balancePrefixComponent
        Text {
            property string balance: "1 000"
            text: (optionsModel.displayUnit === 1 ? "s" : "₿") + " " + balance
        }
    }

    function test_walletBadge_prefix_is_s_in_sat_mode() {
        optionsModel.displayUnit = 1
        const obj = createTemporaryObject(balancePrefixComponent, this)
        verify(obj !== null)
        compare(obj.text, "s 1 000")
        optionsModel.displayUnit = 0
    }

    function test_walletBadge_prefix_is_btc_symbol_in_btc_mode() {
        optionsModel.displayUnit = 0
        const obj = createTemporaryObject(balancePrefixComponent, this)
        verify(obj !== null)
        compare(obj.text, "₿ 1 000")
    }

    // Mirrors the Loader + Component pattern in BitcoinAmountInputField.qml,
    // Send.qml, and RequestPayment.qml.
    Component {
        id: unitLabelComponent
        Item {
            id: wrapper
            property int unit: 0
            Loader {
                objectName: "unitLabelLoader"
                sourceComponent: wrapper.unit === 1 ? satComponent : btcComponent
            }
            Component { id: btcComponent; Text { text: "₿" } }
            Component { id: satComponent; Text { text: "s" } }
        }
    }

    function test_unitLabel_shows_s_in_sat_mode() {
        const obj = createTemporaryObject(unitLabelComponent, this)
        obj.unit = 1
        waitForRendering(obj)
        const loader = findChild(obj, "unitLabelLoader")
        verify(loader.item !== null)
        compare(loader.item.text, "s")
    }

    function test_unitLabel_shows_btc_symbol_in_btc_mode() {
        const obj = createTemporaryObject(unitLabelComponent, this)
        obj.unit = 0
        waitForRendering(obj)
        const loader = findChild(obj, "unitLabelLoader")
        verify(loader.item !== null)
        compare(loader.item.text, "₿")
    }
}
