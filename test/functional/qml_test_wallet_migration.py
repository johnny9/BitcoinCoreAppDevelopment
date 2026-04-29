#!/usr/bin/env python3
# Copyright (c) 2026 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""End-to-end GUI test for selector-driven legacy wallet migration."""

import os
import shutil
import sys
import time

from qml_test_harness import dump_qml_tree
from qml_wallet_test_lib import WalletFlowHarness, find_legacy_bitcoind, rpc_call


def wait_for_migrated_wallet(rpc_port, wallet_name, timeout=60):
    deadline = time.time() + timeout
    last_error = None
    while time.time() < deadline:
        try:
            wallet_info = rpc_call(rpc_port, "getwalletinfo", wallet=wallet_name, timeout=5)
            if wallet_info.get("descriptors") is True and wallet_info.get("format") == "sqlite":
                return wallet_info
        except Exception as err:  # noqa: BLE001 - retry while the GUI migrates and loads the wallet
            last_error = err
        time.sleep(0.25)
    raise AssertionError(f"Timed out waiting for migrated wallet {wallet_name}: {last_error}")


def run_test():
    harness = WalletFlowHarness("qml_wallet_migration", port_offset=50)
    gui = None
    try:
        legacy_binary = find_legacy_bitcoind()
        if not legacy_binary:
            print("SKIPPED: legacy bitcoind not found.")
            print("Set BITCOIND_LEGACY or provide releases/v28.0/bin/bitcoind to exercise this flow.")
            return 77

        harness.start_source_node(
            binary=legacy_binary,
            extra_args=["-deprecatedrpc=create_bdb"],
        )
        wallet_name = "legacy_flow"
        try:
            rpc_call(
                harness.source_rpc_port,
                "createwallet",
                {
                    "wallet_name": wallet_name,
                    "descriptors": False,
                },
            )
        except RuntimeError as err:
            print(f"SKIPPED: could not create a legacy wallet fixture: {err}")
            return 77

        rpc_call(harness.source_rpc_port, "unloadwallet", [wallet_name])
        source_wallet_path = os.path.join(harness.source_wallets_path, wallet_name)
        target_wallet_path = os.path.join(harness.gui_wallets_path, wallet_name)
        os.makedirs(os.path.dirname(target_wallet_path), exist_ok=True)
        shutil.copytree(source_wallet_path, target_wallet_path)
        harness.stop_source_node()

        harness.start_gui()
        gui = harness.driver
        gui.wait_for_property("walletBadge", "loading", False, timeout_ms=20000)
        gui.wait_for_property("walletBadge", "visible", True, timeout_ms=10000)

        wallet_dir = rpc_call(harness.gui_rpc_port, "listwalletdir")["wallets"]
        legacy_entry = next((entry for entry in wallet_dir if entry["name"] == wallet_name), None)
        assert legacy_entry is not None, f"Expected {wallet_name} to be present in listwalletdir"
        assert legacy_entry["warnings"] == [
            "This wallet is a legacy wallet and will need to be migrated with migratewallet before it can be loaded"
        ], f"Expected migration warning for {wallet_name}"

        gui.click("walletBadge")
        gui.wait_for_property("walletSelectPopup", "opened", True, timeout_ms=5000)
        gui.wait_for_property("walletSelectList", "count", 1, timeout_ms=5000)
        gui.wait_for_object(f"walletSelectItem_{wallet_name}", timeout_ms=5000)
        gui.click(f"walletSelectItem_{wallet_name}")

        gui.wait_for_property("walletMigrationPopup", "opened", True, timeout_ms=10000)
        gui.click("walletMigrationConfirmButton")

        wallet_info = wait_for_migrated_wallet(harness.gui_rpc_port, wallet_name)
        wallet_dat_path = os.path.join(target_wallet_path, "wallet.dat")
        with open(wallet_dat_path, "rb") as wallet_file:
            assert wallet_file.read(16) == b"SQLite format 3\x00", "Migrated wallet should be SQLite"

        assert wallet_info["descriptors"] is True, "Migrated wallet should be descriptor based"
        assert wallet_info["format"] == "sqlite", "Migrated wallet should be stored as sqlite"

        print("Migration flow passed.")
        return 0
    except Exception as err:  # noqa: BLE001 - preserve failure context for functional test output
        print(f"\nFAILED: {err}", file=sys.stderr)
        import traceback
        traceback.print_exc()
        gui_output = harness.process_output(harness.gui_process)
        if gui_output:
            print("\n--- GUI process output ---", file=sys.stderr)
            print(gui_output, file=sys.stderr)
        if gui is not None:
            dump_qml_tree(gui)
        return 1
    finally:
        harness.stop()


if __name__ == "__main__":
    sys.exit(run_test())
