#!/usr/bin/env python3
# Copyright (c) 2026 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""End-to-end GUI test for creating an external signer wallet."""

import argparse
import json
import os
import re
import sys
from datetime import datetime

from qml_driver import QmlDriverError
from qml_test_harness import dump_qml_tree
from qml_wallet_test_lib import WalletFlowHarness, rpc_call


EXPECTED_SIGNER_NAME = "trezor_t"
EXPECTED_STATUS_TEXT = f"Detected external signer: {EXPECTED_SIGNER_NAME}"
EXPECTED_FIRST_BECH32_ADDRESS = "bcrt1qm90ugl4d48jv8n6e5t9ln6t9zlpm5th68x4f8g"
EXPECTED_FIRST_HD_KEYPATH = "m/84h/1h/0h/0/0"


def parse_args():
    parser = argparse.ArgumentParser(
        description="External wallet creation GUI functional test",
        add_help=True,
    )
    parser.add_argument(
        "--save-screenshots",
        action="store_true",
        help="Save a PNG at each GUI checkpoint under test/artifacts/",
    )
    return parser.parse_args()


def make_screenshot_root():
    repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
    artifacts_root = os.path.join(repo_root, "test", "artifacts")
    timestamp = datetime.now().strftime("%Y%m%d-%H%M%S")
    screenshot_root = os.path.join(artifacts_root, f"qml_test_external_wallet-{timestamp}")
    os.makedirs(screenshot_root, exist_ok=True)
    return screenshot_root


def find_mock_signer_path():
    repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
    signer_path = os.path.join(repo_root, "bitcoin", "test", "functional", "mocks", "signer.py")
    if not os.path.isfile(signer_path):
        raise FileNotFoundError(f"Mock signer not found at {signer_path}")
    if not os.access(signer_path, os.X_OK):
        raise PermissionError(f"Mock signer is not executable: {signer_path}")
    return signer_path


class CheckpointRecorder:
    def __init__(self, case_name, save_screenshots, screenshot_root):
        self.case_name = case_name
        self.save_screenshots = save_screenshots
        self.screenshot_root = screenshot_root
        self.index = 0

    def _sanitize_label(self, label):
        return re.sub(r"[^a-z0-9]+", "-", label.lower()).strip("-") or "checkpoint"

    def checkpoint(self, label, gui=None):
        self.index += 1
        prefix = f"[{self.case_name}] checkpoint {self.index:02d}"
        print(f"{prefix}: {label}")
        if gui is None:
            return

        gui.settle()

        if not self.save_screenshots:
            return

        case_dir = os.path.join(self.screenshot_root, self.case_name)
        filename = f"{self.index:02d}-{self._sanitize_label(label)}.png"
        screenshot_path = os.path.join(case_dir, filename)
        screenshot = gui.save_screenshot(screenshot_path)
        print(
            f"{prefix}: screenshot saved to {screenshot['path']} "
            f"({screenshot['width']}x{screenshot['height']})"
        )


def open_add_wallet_flow(gui):
    gui.wait_for_property("walletBadge", "loading", False, timeout_ms=20000)
    gui.click("walletBadge")
    try:
        gui.wait_for_property("walletSelectPopup", "opened", True, timeout_ms=2000)
        gui.click("walletSelectAddWalletButton")
    except QmlDriverError:
        pass
    gui.wait_for_property("createWalletButton", "visible", True, timeout_ms=10000)


def run_test(args):
    case_name = "qml_external_wallet_creation"
    screenshot_root = None
    if args.save_screenshots:
        screenshot_root = make_screenshot_root()
        print(f"Checkpoint screenshots will be saved under: {screenshot_root}")

    harness = WalletFlowHarness(case_name, port_offset=70)
    checkpoints = CheckpointRecorder(case_name, args.save_screenshots, screenshot_root)
    gui = None

    try:
        signer_path = find_mock_signer_path()
        print(f"[{case_name}] using mock signer: {signer_path}")

        harness.start_gui()
        gui = harness.driver

        gui.wait_for_property("walletBadge", "loading", False, timeout_ms=20000)
        gui.wait_for_property("walletBadge", "visible", True, timeout_ms=10000)
        assert gui.get_property("walletBadge", "noWalletLoaded") is True, "Expected no wallet to be loaded at startup"
        checkpoints.checkpoint("gui launched", gui)

        gui.click("desktopWalletSettingsTabButton")
        gui.wait_for_property("settingsWallet", "visible", True, timeout_ms=10000)
        gui.click("settingsWallet")
        gui.wait_for_property("externalSignerPathInput", "visible", True, timeout_ms=10000)
        checkpoints.checkpoint("wallet settings opened", gui)

        gui.set_text("externalSignerPathInput", signer_path)
        checkpoints.checkpoint("mock signer path populated", gui)

        gui.click("externalSignerCheckDeviceButton")
        gui.wait_for_property("externalSignerStatusText", "text", EXPECTED_STATUS_TEXT, timeout_ms=10000)
        checkpoints.checkpoint("mock signer detected", gui)
        print(f"[{case_name}] signer status: {gui.get_text('externalSignerStatusText')}")

        open_add_wallet_flow(gui)
        checkpoints.checkpoint("add wallet flow opened", gui)

        gui.wait_for_property("createExternalWalletEntryButton", "visible", True, timeout_ms=10000)
        checkpoints.checkpoint("external wallet option available", gui)

        gui.click("createExternalWalletEntryButton")
        gui.wait_for_property("externalWalletNameInput", "visible", True, timeout_ms=10000)
        gui.wait_for_property("createExternalWalletButton", "enabled", True, timeout_ms=10000)
        checkpoints.checkpoint("external wallet form opened", gui)

        wallet_name = gui.get_text("externalWalletNameInput")
        print(f"[{case_name}] suggested wallet name: {wallet_name}")
        assert wallet_name == EXPECTED_SIGNER_NAME, (
            f"Expected wallet name to default to {EXPECTED_SIGNER_NAME!r}, got {wallet_name!r}"
        )
        checkpoints.checkpoint("wallet name prefilled from signer", gui)

        gui.click("createExternalWalletButton")
        gui.wait_for_page("externalWalletCreatedPage", timeout_ms=20000)
        checkpoints.checkpoint("external wallet created", gui)

        gui.click("externalWalletCreatedDoneButton")
        gui.wait_for_property("walletBadge", "text", wallet_name, timeout_ms=20000)
        checkpoints.checkpoint("wallet overview reopened", gui)

        wallet_dir = os.path.join(harness.gui_wallets_path, wallet_name)
        assert os.path.isdir(wallet_dir), f"Expected wallet directory at {wallet_dir}"
        print(f"[{case_name}] wallet directory: {wallet_dir}")

        wallet_info = rpc_call(harness.gui_rpc_port, "getwalletinfo", wallet=wallet_name)
        print(f"[{case_name}] wallet info: {json.dumps(wallet_info, sort_keys=True)}")
        assert wallet_info["external_signer"] is True, "Expected external_signer wallet flag"
        assert wallet_info["private_keys_enabled"] is False, "External signer wallet should disable private keys"
        assert wallet_info["descriptors"] is True, "External signer wallet should be descriptor based"
        assert wallet_info["format"] == "sqlite", "External signer wallet should use sqlite storage"

        receive_address = rpc_call(
            harness.gui_rpc_port,
            "getnewaddress",
            ["", "bech32"],
            wallet=wallet_name,
        )
        print(f"[{case_name}] first bech32 address: {receive_address}")
        assert receive_address == EXPECTED_FIRST_BECH32_ADDRESS, (
            "Expected upstream mock signer to provide the same first bech32 address "
            "used in bitcoin/test/functional/wallet_signer.py"
        )

        address_info = rpc_call(harness.gui_rpc_port, "getaddressinfo", [receive_address], wallet=wallet_name)
        print(f"[{case_name}] address info: {json.dumps(address_info, sort_keys=True)}")
        assert address_info["solvable"] is True, "Expected signer-derived address to be solvable"
        assert address_info["ismine"] is True, "Expected signer-derived address to be recognized as mine"
        assert address_info["hdkeypath"] == EXPECTED_FIRST_HD_KEYPATH, (
            f"Expected first signer-derived address hdkeypath {EXPECTED_FIRST_HD_KEYPATH}, "
            f"got {address_info['hdkeypath']}"
        )
        checkpoints.checkpoint("external wallet rpc descriptors verified", gui)

        print(f"[{case_name}] completed")
        return 0
    except Exception as err:  # noqa: BLE001 - preserve failure context for functional test output
        print(f"\nFAILED [{case_name}]: {err}", file=sys.stderr)
        import traceback
        traceback.print_exc()
        if gui is not None:
            try:
                checkpoints.checkpoint("failure state", gui)
            except Exception as screenshot_err:  # noqa: BLE001 - preserve original failure context
                print(f"[{case_name}] failed to save failure screenshot: {screenshot_err}", file=sys.stderr)
        gui_output = ""
        if harness.gui_process and harness.gui_process.poll() is not None:
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
    sys.exit(run_test(parse_args()))
