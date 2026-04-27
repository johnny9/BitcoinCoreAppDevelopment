// Copyright (c) 2026 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15
import QtTest 1.2
import "../../qml/components"

TestCase {
    name: "FeeSelection"
    when: windowShown
    width: 900
    height: 700

    Component {
        id: feeSelectionComponent

        FeeSelection {
            walletModel: testWalletModel
            currentTarget: testWalletModel.targetBlocks
            onFeeChanged: function(target) {
                testWalletModel.targetBlocks = target
            }
        }
    }

    SignalSpy {
        id: feeChangedSpy
    }

    SignalSpy {
        id: includeFeeInAmountToggledSpy
    }

    function init() {
        testWalletModel.clearFeeEstimates()
        testWalletModel.setFeeEstimate(1, "0.00000750 ₿")
        testWalletModel.setFeeEstimate(2, "0.00000500 ₿")
        testWalletModel.setFeeEstimate(6, "0.00000250 ₿")
        testWalletModel.customFeeEnabled = false
        testWalletModel.customFeeRate = ""
        testWalletModel.targetBlocks = 2
        includeFeeInAmountToggledSpy.target = null
        includeFeeInAmountToggledSpy.signalName = ""
    }

    function test_feeSelection_has_stable_selectors() {
        const control = createTemporaryObject(feeSelectionComponent, this)
        verify(control !== null)

        compare(control.objectName, "feeSelectionControl")
        verify(findChild(control, "feeSelectionEstimateLabel") !== null)
        verify(findChild(control, "feeSelectionCustomRateInput") !== null)
        verify(findChild(control, "feeSelectionCustomEstimateLabel") !== null)
        verify(findChild(control, "feeSelectionDropdownButton") !== null)
        verify(findChild(control, "feeSelectionPopup") !== null)
        verify(findChild(control, "feeSelectionList") !== null)
        verify(findChild(control, "feeSelectionIncludeFeeToggle") !== null)
    }

    function test_feeSelection_matches_standard_fee_spec_labels_and_estimate() {
        testWalletModel.targetBlocks = 2

        const control = createTemporaryObject(feeSelectionComponent, this)
        verify(control !== null)

        compare(control.selectedIndex, 1)
        compare(control.selectedLabel, "Default")
        compare(control.selectedDuration, "(~20 mins)")
        compare(control.selectedEstimate, "0.00000500 ₿")

        const estimateLabel = findChild(control, "feeSelectionEstimateLabel")
        verify(estimateLabel !== null)
        compare(estimateLabel.text, "0.00000500 ₿")
    }

    function test_feeSelection_selecting_option_updates_state_and_emits() {
        testWalletModel.targetBlocks = 2

        const control = createTemporaryObject(feeSelectionComponent, this)
        verify(control !== null)

        const popup = findChild(control, "feeSelectionPopup")
        const list = findChild(control, "feeSelectionList")
        verify(popup !== null)
        verify(list !== null)

        feeChangedSpy.target = control
        feeChangedSpy.signalName = "feeChanged"
        feeChangedSpy.clear()

        popup.open()
        tryVerify(function() {
            return list.itemAtIndex(2) !== null
        })

        const lowFeeOption = list.itemAtIndex(2)
        verify(lowFeeOption !== null)

        const lowFeeEstimate = findChild(lowFeeOption, "feeSelectionOptionEstimate2")
        verify(lowFeeEstimate !== null)
        compare(lowFeeEstimate.text, "0.00000250 ₿")

        lowFeeOption.clicked()

        compare(control.selectedIndex, 2)
        compare(control.selectedLabel, "Low")
        compare(control.selectedDuration, "(~60 mins)")
        compare(control.selectedEstimate, "0.00000250 ₿")
        compare(feeChangedSpy.count, 1)
        compare(feeChangedSpy.signalArguments[0][0], 6)
        compare(popup.visible, false)
    }

    function test_feeSelection_popup_width_handles_estimate_availability() {
        const control = createTemporaryObject(feeSelectionComponent, this)
        verify(control !== null)

        const popup = findChild(control, "feeSelectionPopup")
        const list = findChild(control, "feeSelectionList")
        verify(popup !== null)
        verify(list !== null)

        popup.open()
        tryVerify(function() {
            return list.itemAtIndex(0) !== null
                && list.itemAtIndex(1) !== null
                && list.itemAtIndex(2) !== null
        })

        const initialWidth = popup.width
        verify(initialWidth > 0)

        testWalletModel.clearFeeEstimates()
        tryVerify(function() {
            return popup.width > 0
        })

        const widthWithoutEstimates = popup.width
        verify(widthWithoutEstimates > 0)

        testWalletModel.setFeeEstimate(2, "12345.12345678 ₿")

        tryVerify(function() {
            return popup.width >= widthWithoutEstimates
        })
    }

    function test_feeSelection_popup_estimates_stay_aligned_when_selection_changes() {
        testWalletModel.targetBlocks = 2

        const control = createTemporaryObject(feeSelectionComponent, this)
        verify(control !== null)

        const popup = findChild(control, "feeSelectionPopup")
        const list = findChild(control, "feeSelectionList")
        verify(popup !== null)
        verify(list !== null)

        popup.open()
        tryVerify(function() {
            return list.itemAtIndex(0) !== null
                && list.itemAtIndex(1) !== null
                && list.itemAtIndex(2) !== null
        })

        const highFeeEstimate = findChild(list.itemAtIndex(0), "feeSelectionOptionEstimate0")
        const defaultFeeEstimate = findChild(list.itemAtIndex(1), "feeSelectionOptionEstimate1")
        const lowFeeEstimate = findChild(list.itemAtIndex(2), "feeSelectionOptionEstimate2")
        verify(highFeeEstimate !== null)
        verify(defaultFeeEstimate !== null)
        verify(lowFeeEstimate !== null)

        compare(highFeeEstimate.x + highFeeEstimate.width, defaultFeeEstimate.x + defaultFeeEstimate.width)
        compare(defaultFeeEstimate.x + defaultFeeEstimate.width, lowFeeEstimate.x + lowFeeEstimate.width)
        verify(highFeeEstimate.width >= highFeeEstimate.contentWidth)
        verify(defaultFeeEstimate.width >= defaultFeeEstimate.contentWidth)
        verify(lowFeeEstimate.width >= lowFeeEstimate.contentWidth)
    }

    function test_feeSelection_current_target_syncs_selected_preset() {
        testWalletModel.targetBlocks = 6

        const control = createTemporaryObject(feeSelectionComponent, this)
        verify(control !== null)

        compare(control.selectedIndex, 2)
        compare(control.selectedLabel, "Low")
        compare(control.selectedDuration, "(~60 mins)")
        compare(control.selectedEstimate, "0.00000250 ₿")

        testWalletModel.targetBlocks = 1
        compare(control.selectedIndex, 0)
        compare(control.selectedLabel, "High")
        compare(control.selectedDuration, "(~10 mins)")
        compare(control.selectedEstimate, "0.00000750 ₿")
    }

    function test_feeSelection_custom_option_switches_to_fee_rate_mode() {
        const control = createTemporaryObject(feeSelectionComponent, this)
        verify(control !== null)

        const popup = findChild(control, "feeSelectionPopup")
        const list = findChild(control, "feeSelectionList")
        const presetEstimateLabel = findChild(control, "feeSelectionEstimateLabel")
        const customFeeRateInput = findChild(control, "feeSelectionCustomRateInput")
        const customEstimateLabel = findChild(control, "feeSelectionCustomEstimateLabel")
        verify(popup !== null)
        verify(list !== null)
        verify(presetEstimateLabel !== null)
        verify(customFeeRateInput !== null)
        verify(customEstimateLabel !== null)

        popup.open()
        tryVerify(function() {
            return list.itemAtIndex(3) !== null
        })

        const customOption = list.itemAtIndex(3)
        verify(customOption !== null)

        const customOptionEstimate = findChild(customOption, "feeSelectionOptionEstimate3")
        verify(customOptionEstimate !== null)
        compare(customOptionEstimate.text, "")

        customOption.clicked()

        compare(control.selectedIndex, 3)
        compare(control.selectedLabel, "sats/vbyte")
        compare(control.selectedDuration, "")
        compare(testWalletModel.customFeeEnabled, true)
        tryCompare(presetEstimateLabel, "visible", false)
        compare(customEstimateLabel.text, "")
    }

    function test_feeSelection_custom_rate_updates_fee_estimate_and_presets_restore_layout() {
        const control = createTemporaryObject(feeSelectionComponent, this)
        verify(control !== null)

        const popup = findChild(control, "feeSelectionPopup")
        const list = findChild(control, "feeSelectionList")
        const presetEstimateLabel = findChild(control, "feeSelectionEstimateLabel")
        const customFeeRateInput = findChild(control, "feeSelectionCustomRateInput")
        const customEstimateLabel = findChild(control, "feeSelectionCustomEstimateLabel")
        verify(popup !== null)
        verify(list !== null)
        verify(presetEstimateLabel !== null)
        verify(customFeeRateInput !== null)
        verify(customEstimateLabel !== null)

        popup.open()
        tryVerify(function() {
            return list.itemAtIndex(3) !== null
        })
        list.itemAtIndex(3).clicked()

        customFeeRateInput.text = "2"
        compare(testWalletModel.customFeeRate, "2")
        compare(testWalletModel.customFeeRateValid, true)
        tryCompare(customEstimateLabel, "text", "0.00000400 ₿")

        popup.open()
        tryVerify(function() {
            return list.itemAtIndex(2) !== null
        })
        list.itemAtIndex(2).clicked()

        compare(testWalletModel.customFeeEnabled, false)
        compare(control.selectedIndex, 2)
        compare(control.selectedLabel, "Low")
        compare(control.selectedEstimate, "0.00000250 ₿")
    }

    function test_feeSelection_include_fee_toggle_tracks_state_and_emits() {
        const control = createTemporaryObject(feeSelectionComponent, this, {
            "includeFeeInAmount": false
        })
        verify(control !== null)

        const popup = findChild(control, "feeSelectionPopup")
        const list = findChild(control, "feeSelectionList")
        const toggle = findChild(control, "feeSelectionIncludeFeeToggle")
        verify(popup !== null)
        verify(list !== null)
        verify(toggle !== null)

        includeFeeInAmountToggledSpy.target = control
        includeFeeInAmountToggledSpy.signalName = "includeFeeInAmountToggled"
        includeFeeInAmountToggledSpy.clear()

        popup.open()
        tryCompare(popup, "opened", true)

        toggle.clicked()

        compare(includeFeeInAmountToggledSpy.count, 1)
        compare(includeFeeInAmountToggledSpy.signalArguments[0][0], true)
        compare(popup.visible, false)
    }
}
