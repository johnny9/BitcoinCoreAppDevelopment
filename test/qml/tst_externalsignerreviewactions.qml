// Copyright (c) 2026 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15
import QtTest 1.2
import "../../qml/components"

TestCase {
    name: "ExternalSignerReviewActions"
    when: windowShown
    width: 500
    height: 300

    Component {
        id: walletComponent

        QtObject {
            property int approveCalls: 0

            signal externalSignerApprovalSucceeded()
            signal externalSignerApprovalFailed(string message, bool signerNotFound)

            function approveExternalSignerTransaction() {
                approveCalls += 1
            }
        }
    }

    Component {
        id: reviewComponent

        ExternalSignerReviewActions {
            width: 420
            buttonObjectName: "externalSignerApproveButton"
            statusObjectName: "externalSignerStatusText"
        }
    }

    function findObjectByName(root, objectName) {
        if (!root) {
            return null
        }
        if (root.objectName === objectName) {
            return root
        }

        if (root.contentItem) {
            const contentResult = findObjectByName(root.contentItem, objectName)
            if (contentResult) {
                return contentResult
            }
        }

        const children = root.children || []
        for (let i = 0; i < children.length; ++i) {
            const childResult = findObjectByName(children[i], objectName)
            if (childResult) {
                return childResult
            }
        }

        return null
    }

    function waitForApproveCall(wallet) {
        tryCompare(wallet, "approveCalls", 1, 1000)
    }

    function test_waiting_state_persists_until_mock_completes() {
        const wallet = createTemporaryObject(walletComponent, this)
        verify(wallet !== null)

        const review = createTemporaryObject(reviewComponent, this, {wallet: wallet})
        verify(review !== null)

        const button = findObjectByName(review, "externalSignerApproveButton")
        verify(button !== null)

        compare(review.reviewState, "initial")
        compare(review.statusText, "Approve on external signer to broadcast this transaction.")
        compare(review.buttonText, "Approve on external signer")
        verify(button.enabled)

        review.beginApproval()

        compare(review.reviewState, "waiting")
        compare(review.statusText, "Waiting for approval on external signer.")
        compare(review.buttonText, "Waiting for approval...")
        verify(!button.enabled)

        waitForApproveCall(wallet)
        compare(review.reviewState, "waiting")

        wallet.externalSignerApprovalSucceeded()

        compare(review.reviewState, "signed")
        compare(review.statusText, "Signed on external signer. Ready to send.")
        compare(review.buttonText, "Send")
        verify(button.enabled)
    }
}
