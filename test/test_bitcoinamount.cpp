// Copyright (c) 2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QtTest/QtTest>

#include <qml/bitcoinamount.h>

class BitcoinAmountTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void satsToBtcString_basic();
    void satsToBtcString_negative();
    void btcToSats_roundtrip();
    void sanitize_clampsAndFilters();
    void display_flow_btc();
    void display_flow_sat();
    void flipUnit_changesLabelAndDisplaySignal();
};

void BitcoinAmountTests::satsToBtcString_basic()
{
    QCOMPARE(BitcoinAmount::satsToBtcString(0), QString("0.00000000"));
    QCOMPARE(BitcoinAmount::satsToBtcString(1), QString("0.00000001"));
    QCOMPARE(BitcoinAmount::satsToBtcString(COIN), QString("1.00000000"));
    QCOMPARE(BitcoinAmount::satsToBtcString(21 * COIN), QString("21.00000000"));
    QCOMPARE(BitcoinAmount::satsToBtcString(100000123), QString("1.00000123"));
}

void BitcoinAmountTests::satsToBtcString_negative()
{
    QCOMPARE(BitcoinAmount::satsToBtcString(-1), QString("-0.00000001"));
}

void BitcoinAmountTests::btcToSats_roundtrip()
{
    BitcoinAmount amt;
    amt.setUnit(BitcoinAmount::Unit::BTC);
    amt.fromDisplay("1.23456789");
    QCOMPARE(amt.toDisplay(), QString("1.23456789"));
}

void BitcoinAmountTests::sanitize_clampsAndFilters()
{
    BitcoinAmount amt;
    amt.setUnit(BitcoinAmount::Unit::BTC);
    amt.fromDisplay("abc1.2.3xyz");
    QCOMPARE(amt.toDisplay(), QString("1.20000000"));
}

void BitcoinAmountTests::display_flow_btc()
{
    BitcoinAmount amt;
    amt.setUnit(BitcoinAmount::Unit::BTC);
    amt.setSatoshi(2 * COIN);
    QCOMPARE(amt.toDisplay(), QString("2.00000000"));
}

void BitcoinAmountTests::display_flow_sat()
{
    BitcoinAmount amt;
    amt.setUnit(BitcoinAmount::Unit::SAT);
    amt.setSatoshi(123);
    QCOMPARE(amt.toDisplay(), QString("123"));
}

void BitcoinAmountTests::flipUnit_changesLabelAndDisplaySignal()
{
    BitcoinAmount amt;
    amt.setSatoshi(100);
    QSignalSpy spy(&amt, &BitcoinAmount::displayChanged);
    amt.flipUnit();
    QVERIFY(spy.count() >= 1);
}

#ifdef BITCOINQML_NO_TEST_MAIN
#include <test/qt_test_registry.h>
BITCOINQML_REGISTER_QT_TEST(BitcoinAmountTests)
#else
QTEST_MAIN(BitcoinAmountTests)
#endif
#include "test_bitcoinamount.moc"
