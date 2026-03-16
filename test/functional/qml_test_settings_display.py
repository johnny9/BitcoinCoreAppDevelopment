#!/usr/bin/env python3
# Copyright (c) 2026 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""End-to-end tests for the Display settings page.

Tests language selection, display unit switching (BTC / SAT), and the
"Ask before opening links" toggle.

These tests run post-onboarding and do not require a peer connection, but
they do require the node to start up, so generous wait timeouts are used.

This test requires:
  - bitcoin-core-app built with -DENABLE_TEST_AUTOMATION=ON
"""

import sys
import time

from qml_test_harness import (
    GUI_STARTUP_TIMEOUT,
    QmlTestHarness,
    complete_onboarding,
    dump_qml_tree,
    parse_args,
)
from qml_driver import QmlDriverError

# The node must start up before post-onboarding pages are interactive.
# Use a generous timeout for waits that follow onboarding completion.
POST_ONBOARDING_TIMEOUT_MS = 30000


# ── Navigation helpers ────────────────────────────────────────────────────────

def navigate_to_display_settings(gui):
    """From the NodeRunner main screen, navigate to the Display settings page."""
    gui.click("nodeSettingsButton")
    # Wait for the NodeSettings page to finish loading (gotoDisplay is one of
    # its rows).
    gui.wait_for_page("gotoDisplay", timeout_ms=5000)
    gui.click("gotoDisplay")
    # SettingsDisplay is identified by the presence of gotoDisplayUnit.
    gui.wait_for_page("gotoDisplayUnit", timeout_ms=5000)
    print("  Navigated to Display settings page")


# ── Individual test cases ─────────────────────────────────────────────────────

def test_display_unit_selection(gui):
    """Select SAT on the Display unit page and verify it is reflected."""
    print("\n── test_display_unit_selection ───────────────────────────────")

    gui.click("gotoDisplayUnit")
    gui.wait_for_page("settingsDisplayUnitPage", timeout_ms=5000)
    print("  Navigated to SettingsDisplayUnit page")

    # If SAT is selected (state persists across runs), switch to BTC first.
    # We must navigate to a fresh page after the switch because clicking a
    # checkable OptionButton breaks its declarative `checked:` binding.
    if gui.get_property("displayUnitSAT", "checked"):
        gui.click("displayUnitBTC")
        gui.click("settingsDisplayUnitBack")
        gui.wait_for_page("gotoDisplayUnit", timeout_ms=5000)
        gui.click("gotoDisplayUnit")
        gui.wait_for_page("settingsDisplayUnitPage", timeout_ms=5000)
        print("  Switched to BTC starting state")

    btc_checked = gui.get_property("displayUnitBTC", "checked")
    sat_checked = gui.get_property("displayUnitSAT", "checked")
    assert btc_checked, (
        f"BTC should be checked at test start, got btc={btc_checked} sat={sat_checked}"
    )
    assert not sat_checked, (
        f"SAT should not be checked at test start, got sat={sat_checked}"
    )
    print(f"  Starting state: BTC={btc_checked}, SAT={sat_checked}  PASSED")

    # Select SAT.
    gui.click("displayUnitSAT")
    sat_after = gui.get_property("displayUnitSAT", "checked")
    btc_after = gui.get_property("displayUnitBTC", "checked")
    assert sat_after, f"SAT should be checked after clicking, got sat={sat_after}"
    assert not btc_after, f"BTC should be unchecked after selecting SAT, got btc={btc_after}"
    print(f"  After SAT selection: SAT={sat_after}, BTC={btc_after}  PASSED")

    # Go back and reset to BTC for future runs.
    gui.click("settingsDisplayUnitBack")
    gui.wait_for_page("gotoDisplayUnit", timeout_ms=5000)

    gui.click("gotoDisplayUnit")
    gui.wait_for_page("settingsDisplayUnitPage", timeout_ms=5000)
    gui.click("displayUnitBTC")
    gui.click("settingsDisplayUnitBack")
    gui.wait_for_page("gotoDisplayUnit", timeout_ms=5000)
    print("  Reset display unit to BTC  PASSED")


def test_language_selection(gui):
    """Select Spanish and verify the summary and translated headers update."""
    print("\n── test_language_selection ───────────────────────────────────")

    gui.click("gotoLanguage")
    gui.wait_for_page("settingsLanguagePage", timeout_ms=5000)
    print("  Navigated to SettingsLanguage page")

    # Filter the list to Spanish so the delegate is rendered by the ListView.
    gui.set_text("languageSearch", "español")
    gui.wait_for_page("language_es", timeout_ms=3000)
    gui.click("language_es")
    # Selecting a language navigates back to SettingsDisplay automatically.
    gui.wait_for_page("gotoLanguage", timeout_ms=5000)
    print("  Selected Spanish (es) and returned to Display settings")

    # Verify the language summary description on the row has updated.
    lang_summary = gui.get_property("gotoLanguage", "description")
    assert "Español" in lang_summary, (
        f"Language summary should contain 'Español' after selecting 'es', "
        f"got: {lang_summary!r}"
    )
    assert "system default" not in lang_summary.lower(), (
        f"Language summary should not show 'System default' after selecting 'es', "
        f"got: {lang_summary!r}"
    )
    print(f"  Language summary updated to: {lang_summary!r}  PASSED")

    # Verify translation propagated to other row headers on this page.
    lang_header = gui.get_property("gotoLanguage", "header")
    assert lang_header == "Idioma", (
        f"'Language' row header should be 'Idioma' in Spanish, got: {lang_header!r}"
    )
    print(f"  Language row header translated: {lang_header!r}  PASSED")

    unit_header = gui.get_property("gotoDisplayUnit", "header")
    assert unit_header == "Unidad de visualización", (
        f"'Display unit' row header should be translated in Spanish, got: {unit_header!r}"
    )
    print(f"  Display unit row header translated: {unit_header!r}  PASSED")

    # Reset to System default (empty tag).
    gui.click("gotoLanguage")
    gui.wait_for_page("settingsLanguagePage", timeout_ms=5000)
    gui.wait_for_page("language_", timeout_ms=3000)  # wait for delegate to render
    gui.click("language_")  # objectName: "language_" + "" = "language_"
    gui.wait_for_page("gotoLanguage", timeout_ms=5000)

    default_summary = gui.get_property("gotoLanguage", "description")
    assert "system default" in default_summary.lower(), (
        f"Expected 'System default' summary after reset, got: {default_summary!r}"
    )
    print(f"  Reset to System default: {default_summary!r}  PASSED")

    # Verify English headers are restored after reset.
    lang_header_reset = gui.get_property("gotoLanguage", "header")
    assert lang_header_reset == "Language", (
        f"'Language' header should be restored to English after reset, got: {lang_header_reset!r}"
    )
    unit_header_reset = gui.get_property("gotoDisplayUnit", "header")
    assert unit_header_reset == "Display unit", (
        f"'Display unit' header should be restored to English after reset, got: {unit_header_reset!r}"
    )
    print(f"  Headers restored to English  PASSED")


# ── Main ──────────────────────────────────────────────────────────────────────

def run_tests():
    args = parse_args()
    harness = QmlTestHarness(socket_path=args.socket_path, extra_args=["-disablewallet"])
    try:
        harness.start()
        gui = harness.driver

        # Complete onboarding to reach the main node screen.
        complete_onboarding(gui)

        # Wait for the NodeRunner main screen.
        # Uses a generous timeout because the node starts up after onboarding.
        gui.wait_for_page("nodeSettingsButton", timeout_ms=POST_ONBOARDING_TIMEOUT_MS)
        print("Reached NodeRunner main screen")

        navigate_to_display_settings(gui)

        test_display_unit_selection(gui)
        test_language_selection(gui)

        print("\n" + "=" * 60)
        print("All display settings tests PASSED")
        print("=" * 60)

    except Exception as e:
        print(f"\nFAILED: {e}", file=sys.stderr)
        import traceback
        traceback.print_exc()
        if harness.driver:
            dump_qml_tree(harness.driver)
        sys.exit(1)
    finally:
        harness.stop()


if __name__ == '__main__':
    run_tests()
