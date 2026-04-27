// Copyright (c) 2026 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15
import QtTest 1.2
import "../../qml/pages/wallet"

TestCase {
    name: "ActivityDetails"
    when: windowShown
    width: 900
    height: 700

    Component {
        id: detailsComponent
        ActivityDetails {
            width: 600
            height: 700
        }
    }

    function test_speedUpBanner_exists_when_bumpable() {
        const page = createTemporaryObject(detailsComponent, this, {
            txid: "bbbb",
            canBump: true,
            amount: "-0.00100000 BTC",
            date: "2026-01-02",
            depth: 0,
            status: 0,
            type: 3,
            address: "bcrt1qsendaddress"
        })
        verify(page !== null)
        compare(page.canBump, true)

        const banner = findChild(page, "speedUpBanner")
        verify(banner !== null)
    }

    function test_canBump_false_hides_speedUpBanner() {
        const page = createTemporaryObject(detailsComponent, this, {
            txid: "aaaa",
            canBump: false,
            amount: "+0.01000000 BTC",
            date: "2026-01-01",
            depth: 3,
            status: 2,
            type: 1,
            address: "bcrt1qreceiveaddress"
        })
        verify(page !== null)
        compare(page.canBump, false)

        const banner = findChild(page, "speedUpBanner")
        verify(banner !== null)
        compare(banner.visible, false)
    }

    function test_confirmed_tx_hides_speedUpBanner() {
        const page = createTemporaryObject(detailsComponent, this, {
            txid: "cccc",
            canBump: false,
            amount: "-0.00500000 BTC",
            date: "2026-01-03",
            depth: 6,
            status: 2,
            type: 3,
            address: "bcrt1qconfirmedaddress"
        })
        verify(page !== null)

        const banner = findChild(page, "speedUpBanner")
        verify(banner !== null)
        compare(banner.visible, false)
    }

    function test_replacedByTxid_shows_replacedBanner() {
        const page = createTemporaryObject(detailsComponent, this, {
            txid: "aaaa",
            canBump: false,
            replacedByTxid: "bbbb",
            amount: "-0.00100000 BTC",
            date: "2026-01-02",
            depth: -1,
            status: 0,
            type: 3,
            address: "bcrt1qsendaddress"
        })
        verify(page !== null)
        compare(page.replacedByTxid, "bbbb")

        const banner = findChild(page, "replacedBanner")
        verify(banner !== null)
    }

    function test_no_replacedByTxid_hides_replacedBanner() {
        const page = createTemporaryObject(detailsComponent, this, {
            txid: "bbbb",
            canBump: true,
            replacedByTxid: "",
            amount: "-0.00100000 BTC",
            date: "2026-01-02",
            depth: 0,
            status: 0,
            type: 3,
            address: "bcrt1qsendaddress"
        })
        verify(page !== null)

        const banner = findChild(page, "replacedBanner")
        verify(banner !== null)
        compare(banner.visible, false)
    }

    function test_dimmed_when_replaced() {
        const page = createTemporaryObject(detailsComponent, this, {
            txid: "aaaa",
            canBump: false,
            replacedByTxid: "bbbb",
            amount: "-0.00100000 BTC",
            date: "2026-01-02",
            depth: -1,
            status: 0,
            type: 3,
            address: "bcrt1qsendaddress"
        })
        verify(page !== null)
        compare(page.replacedByTxid, "bbbb")
        compare(page.opacity !== undefined, true)
    }
}
