#!/usr/bin/env python3
# Copyright (c) 2026 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test the in-app Debug Log viewer.

Walks through Settings → Debug Log and verifies:
  1. The viewer page loads with a visible search bar and log list.
  2. The log list is non-empty (entries were actually loaded).
  3. Typing in the search field filters the list; clearing it restores
     the full count.

This test requires the binary to be built with -DENABLE_TEST_AUTOMATION=ON.
"""

import datetime
import os
import shutil
import signal
import subprocess
import sys
import tempfile

from qml_test_harness import GUI_STARTUP_TIMEOUT, dump_qml_tree, find_gui_binary, setup_datadir
from qml_driver import QmlDriver


# ── Harness ───────────────────────────────────────────────────────────────────

class DebugLogHarness:
    """Launches the GUI node without -resetguisettings so the app starts
    directly on NodeRunner (not the onboarding wizard).

    bitcoin.cpp sets needOnboarding=true whenever -resetguisettings is
    passed, which sends the app to the OnboardingWizard before NodeRunner.
    Since the debug log test only needs to navigate within a running node,
    we omit that flag and rely on the datadir existing to skip onboarding.
    """

    def __init__(self):
        self.gui_binary = find_gui_binary()
        self.tmpdir = tempfile.mkdtemp(prefix="qml_test_debug_log_")
        self.socket_path = os.path.join(self.tmpdir, "test_bridge.sock")
        self.process = None
        self.driver = None
        self.datadir = setup_datadir(self.tmpdir)

    def start(self):
        env = dict(os.environ)
        env["QT_QPA_PLATFORM"] = "offscreen"
        args = [
            self.gui_binary,
            f"-datadir={self.datadir}",
            f"-test-automation={self.socket_path}",
            # Intentionally omit -resetguisettings: the datadir already
            # exists so needOnboarding stays false and NodeRunner loads first.
            # -disablewallet forces AppMode.walletEnabled=false so main.qml
            # routes to the node/NodeRunner stack instead of desktopWallets.
            "-disablewallet",
            "-logtimemicros",
            "-debug",
            "-debugexclude=libevent",
            "-debugexclude=leveldb",
            "-nolisten",
        ]
        print(f"Starting GUI: {' '.join(args)}")
        self.process = subprocess.Popen(
            args, env=env,
            stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        )
        self.driver = QmlDriver(self.socket_path, timeout=GUI_STARTUP_TIMEOUT)
        print("QmlDriver connected to test bridge.")

    def stop(self):
        if self.process and self.process.poll() is None:
            self.process.send_signal(signal.SIGTERM)
            try:
                self.process.wait(timeout=10)
            except subprocess.TimeoutExpired:
                self.process.kill()
                self.process.wait()
        if self.driver:
            self.driver.close()
        if self.tmpdir:
            shutil.rmtree(self.tmpdir, ignore_errors=True)
            self.tmpdir = None


# ── Navigation ────────────────────────────────────────────────────────────────

def navigate_to_debug_log(gui):
    """From the NodeRunner main screen, navigate to the Debug Log page.

    Requires -disablewallet so AppMode.walletEnabled is false and main.qml
    routes to the node/NodeRunner stack rather than desktopWallets.
    """
    gui.wait_for_page("nodeRunner", timeout_ms=10000)
    gui.click("nodeSettingsButton")
    gui.wait_for_page("nodeSettingsStack", timeout_ms=5000)
    gui.click("settingsDebugLog")
    gui.wait_for_page("settingsDebugLog", timeout_ms=5000)


# ── Test cases ────────────────────────────────────────────────────────────────

def test_viewer_visibility(gui):
    """Verify the search bar and log list are visible on the debug log page."""
    print("\n── test_viewer_visibility ────────────────────────────────────────")
    assert gui.get_property("debugLogSearchField", "visible"), \
        "debugLogSearchField is not visible"
    assert gui.get_property("debugLogListView", "visible"), \
        "debugLogListView is not visible"
    print("  PASSED: search bar and log list are visible")


def test_log_has_entries(gui):
    """Verify that log entries were actually loaded into the list view."""
    print("\n── test_log_has_entries ──────────────────────────────────────────")
    count = gui.get_property("debugLogListView", "count")
    assert count > 0, f"Expected log entries to be loaded, got count={count}"
    print(f"  PASSED: debugLogListView.count = {count}")
    return count


def test_refresh_button(gui, original_count):
    """Clicking refresh reloads the log without dropping existing entries."""
    print("\n── test_refresh_button ───────────────────────────────────────────")
    gui.click("debugLogRefreshButton")
    count = gui.wait_for_property(
        "debugLogListView", "count", lambda c: c >= original_count, timeout_ms=3000
    )
    assert count >= original_count, (
        f"Expected count >= {original_count} after refresh, got {count}"
    )
    print(f"  PASSED: count after refresh = {count}")
    return count


def test_auto_refresh(gui, datadir, current_count):
    """Appending to debug.log triggers auto-refresh and grows the entry count."""
    print("\n── test_auto_refresh ─────────────────────────────────────────────")
    log_path = os.path.join(datadir, "regtest", "debug.log")
    ts = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    marker = f"{ts} test-automation auto-refresh marker"
    with open(log_path, "a", encoding="utf-8") as f:
        f.write(marker + "\n")
    new_count = gui.wait_for_property(
        "debugLogListView", "count", lambda c: c > current_count, timeout_ms=5000
    )
    assert new_count > current_count, (
        f"Expected count to grow beyond {current_count} after appending to debug.log, "
        f"got {new_count}"
    )
    print(f"  PASSED: count grew from {current_count} to {new_count} after file append")


def test_back_navigation(gui):
    """Clicking Back returns to the node settings page."""
    print("\n── test_back_navigation ──────────────────────────────────────────")
    gui.click("debugLogBackButton")
    gui.wait_for_page("nodeSettingsStack", timeout_ms=5000)
    print("  PASSED: back navigation returned to nodeSettingsStack")


def test_search_filter(gui, total_count):
    """Typing in the search field filters the list; clearing it restores all entries."""
    print("\n── test_search_filter ────────────────────────────────────────────")

    # "Bitcoin" appears in the startup banner ("Bitcoin Core version ...") so
    # it is guaranteed to match some lines but very likely not all of them.
    gui.set_text("debugLogSearchField", "Bitcoin")
    # Wait for the debounce timer (150 ms) to propagate searchFilter to the model.
    filtered = gui.wait_for_property(
        "debugLogListView", "count", lambda c: c < total_count
    )
    assert 0 < filtered < total_count, (
        f"Expected a non-zero subset of {total_count} lines after filtering for "
        f"'Bitcoin', got {filtered}"
    )
    print(f"  Filtered count ('Bitcoin'): {filtered} / {total_count}")

    # Clear the search — full list should be restored.
    gui.set_text("debugLogSearchField", "")
    restored = gui.wait_for_property(
        "debugLogListView", "count", lambda c: c == total_count
    )
    assert restored == total_count, (
        f"Expected count to restore to {total_count} after clearing filter, "
        f"got {restored}"
    )
    print(f"  Restored count (cleared): {restored}")
    print("  PASSED: search filter works correctly")


# ── Entry point ───────────────────────────────────────────────────────────────

def run_tests():
    harness = DebugLogHarness()
    try:
        harness.start()
        gui = harness.driver

        print("\nNavigating to Debug Log ...")
        navigate_to_debug_log(gui)
        print(f"  -> page: {gui.get_current_page()}")

        test_viewer_visibility(gui)
        total = test_log_has_entries(gui)
        test_search_filter(gui, total)
        total = test_refresh_button(gui, total)
        test_auto_refresh(gui, harness.datadir, total)
        test_back_navigation(gui)

        print("\n" + "=" * 50)
        print("All debug log tests PASSED")
        print("=" * 50)

    except Exception as e:
        print(f"\nFAILED: {e}", file=sys.stderr)
        import traceback
        traceback.print_exc()
        if harness.process:
            try:
                harness.process.send_signal(signal.SIGTERM)
                try:
                    stderr_bytes = harness.process.communicate(timeout=5)[1]
                except Exception:
                    harness.process.kill()
                    stderr_bytes = harness.process.communicate()[1]
                if stderr_bytes:
                    print("\n--- GUI stderr ---", file=sys.stderr)
                    print(stderr_bytes.decode("utf-8", errors="replace")[-4000:],
                          file=sys.stderr)
            except Exception:
                pass
        if harness.driver:
            dump_qml_tree(harness.driver)
        sys.exit(1)
    finally:
        harness.stop()


if __name__ == '__main__':
    run_tests()
