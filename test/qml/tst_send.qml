// Copyright (c) 2026 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15
import QtTest 1.2
import "../../qml/pages/wallet"

TestCase {
    name: "Send"
    when: windowShown
    width: 900
    height: 700

    Component {
        id: sendComponent

        Send {}
    }

    SignalSpy {
        id: transactionPreparedSpy
    }

    function init() {
        testWalletModel.customFeeEnabled = false
        testWalletModel.customFeeRate = ""
        testWalletModel.targetBlocks = 2
        testWalletModel.prepareTransactionResult = true
        testSendRecipient.subtractFeeFromAmount = false
        testSendRecipient.isValid = true
    }

    function test_send_has_stable_selectors() {
        const page = createTemporaryObject(sendComponent, this)
        verify(page !== null)

        compare(page.objectName, "walletSendPage")
        verify(findChild(page, "sendAddressInput") !== null)
        verify(findChild(page, "sendAmountInput") !== null)
        verify(findChild(page, "sendNoteInput") !== null)
        verify(findChild(page, "feeSelectionEstimateLabel") !== null)
        verify(findChild(page, "feeSelectionCustomRateInput") !== null)
        verify(findChild(page, "feeSelectionCustomEstimateLabel") !== null)
        verify(findChild(page, "feeSelectionIncludeFeeToggle") !== null)
        verify(findChild(page, "sendFeeIncludedNote") !== null)
        verify(findChild(page, "sendFeeIncludedNoteText") !== null)
        verify(findChild(page, "sendContinueButton") !== null)
    }

    function test_send_continue_button_tracks_recipient_validity() {
        const page = createTemporaryObject(sendComponent, this)
        verify(page !== null)

        const continueButton = findChild(page, "sendContinueButton")
        verify(continueButton !== null)

        testSendRecipient.isValid = false
        tryCompare(continueButton, "enabled", false)

        testSendRecipient.isValid = true
        tryCompare(continueButton, "enabled", true)
    }

    function test_send_prepare_transaction_success_and_failure_paths() {
        const page = createTemporaryObject(sendComponent, this)
        verify(page !== null)

        const continueButton = findChild(page, "sendContinueButton")
        verify(continueButton !== null)

        testSendRecipient.isValid = true
        transactionPreparedSpy.target = page
        transactionPreparedSpy.signalName = "transactionPrepared"
        transactionPreparedSpy.clear()

        const callsBefore = testWalletModel.prepareTransactionCalls

        testWalletModel.prepareTransactionResult = false
        continueButton.clicked()
        compare(testWalletModel.prepareTransactionCalls, callsBefore + 1)
        compare(transactionPreparedSpy.count, 0)

        testWalletModel.prepareTransactionResult = true
        continueButton.clicked()
        compare(testWalletModel.prepareTransactionCalls, callsBefore + 2)
        compare(transactionPreparedSpy.count, 1)
        compare(transactionPreparedSpy.signalArguments[0][0], false)
    }

    function test_send_fee_selection_updates_wallet_target_blocks() {
        const page = createTemporaryObject(sendComponent, this)
        verify(page !== null)

        testWalletModel.targetBlocks = 2
        const popup = findChild(page, "feeSelectionPopup")
        const list = findChild(page, "feeSelectionList")
        verify(popup !== null)
        verify(list !== null)

        popup.open()
        tryVerify(function() {
            return list.itemAtIndex(0) !== null
        })

        const highFeeOption = list.itemAtIndex(0)
        verify(highFeeOption !== null)
        highFeeOption.clicked()

        compare(testWalletModel.targetBlocks, 1)
    }

    function test_send_amount_changes_schedule_live_fee_estimation() {
        const page = createTemporaryObject(sendComponent, this)
        verify(page !== null)

        const amountInput = findChild(page, "sendAmountInput")
        verify(amountInput !== null)

        const callsBefore = testWalletModel.scheduleFeeEstimatesCalls
        amountInput.text = "0.01000000"

        tryCompare(testWalletModel, "scheduleFeeEstimatesCalls", callsBefore + 1)
    }

    function test_send_shows_selected_estimated_fee() {
        testWalletModel.targetBlocks = 2

        const page = createTemporaryObject(sendComponent, this)
        verify(page !== null)

        const estimateLabel = findChild(page, "feeSelectionEstimateLabel")
        verify(estimateLabel !== null)
        compare(estimateLabel.text, "0.00000500 ₿")
    }

    function test_send_custom_fee_selection_updates_wallet_mode() {
        const page = createTemporaryObject(sendComponent, this)
        verify(page !== null)

        const popup = findChild(page, "feeSelectionPopup")
        const list = findChild(page, "feeSelectionList")
        const customInput = findChild(page, "feeSelectionCustomRateInput")
        verify(popup !== null)
        verify(list !== null)
        verify(customInput !== null)

        popup.open()
        tryVerify(function() {
            return list.itemAtIndex(3) !== null
        })
        list.itemAtIndex(3).clicked()

        compare(testWalletModel.customFeeEnabled, true)

        popup.open()
        tryVerify(function() {
            return list.itemAtIndex(0) !== null
        })
        list.itemAtIndex(0).clicked()

        compare(testWalletModel.customFeeEnabled, false)
        compare(testWalletModel.targetBlocks, 1)
    }

    function test_send_continue_button_requires_valid_custom_fee() {
        const page = createTemporaryObject(sendComponent, this)
        verify(page !== null)

        const popup = findChild(page, "feeSelectionPopup")
        const list = findChild(page, "feeSelectionList")
        const continueButton = findChild(page, "sendContinueButton")
        const customInput = findChild(page, "feeSelectionCustomRateInput")
        verify(popup !== null)
        verify(list !== null)
        verify(continueButton !== null)
        verify(customInput !== null)

        testSendRecipient.isValid = true

        popup.open()
        tryVerify(function() {
            return list.itemAtIndex(3) !== null
        })
        list.itemAtIndex(3).clicked()

        compare(continueButton.enabled, false)

        customInput.text = "2"
        tryCompare(continueButton, "enabled", true)
    }

    function test_send_include_fee_note_tracks_recipient_state() {
        const page = createTemporaryObject(sendComponent, this)
        verify(page !== null)

        const includedFeeNote = findChild(page, "sendFeeIncludedNote")
        const includedFeeNoteText = findChild(page, "sendFeeIncludedNoteText")
        verify(includedFeeNote !== null)
        verify(includedFeeNoteText !== null)

        compare(includedFeeNote.visible, false)
        compare(includedFeeNoteText.text, "Fees are included in the amount")
    }

    function test_send_include_fee_toggle_updates_recipient_and_schedules_fee_estimate() {
        const page = createTemporaryObject(sendComponent, this)
        verify(page !== null)

        const popup = findChild(page, "feeSelectionPopup")
        const toggle = findChild(page, "feeSelectionIncludeFeeToggle")
        const note = findChild(page, "sendFeeIncludedNote")
        verify(popup !== null)
        verify(toggle !== null)
        verify(note !== null)

        const callsBefore = testWalletModel.scheduleFeeEstimatesCalls

        popup.open()
        tryCompare(popup, "opened", true)

        toggle.clicked()

        compare(testSendRecipient.subtractFeeFromAmount, true)
        tryCompare(testWalletModel, "scheduleFeeEstimatesCalls", callsBefore + 1)
        compare(popup.visible, false)
    }
}
