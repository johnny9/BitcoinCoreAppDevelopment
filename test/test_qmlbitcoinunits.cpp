// Copyright (c) 2026 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QtTest/QtTest>

#include <qml/bitcoinunits.h>

#include <consensus/amount.h>

class QmlBitcoinUnitsTests : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void format_btc_basic();
    void format_btc_negative();
    void format_btc_thinSpaceSeparators();
    void format_sat_noDecimals();
};

void QmlBitcoinUnitsTests::format_btc_basic()
{
    QCOMPARE(QmlBitcoinUnits::format(QmlBitcoinUnits::Unit::BTC, 0), QString("0.00000000"));
    QCOMPARE(QmlBitcoinUnits::format(QmlBitcoinUnits::Unit::BTC, COIN), QString("1.00000000"));
    QCOMPARE(QmlBitcoinUnits::format(QmlBitcoinUnits::Unit::BTC, 123456789), QString("1.23456789"));
}

void QmlBitcoinUnitsTests::format_btc_negative()
{
    QCOMPARE(QmlBitcoinUnits::format(QmlBitcoinUnits::Unit::BTC, -1), QString("-0.00000001"));
}

void QmlBitcoinUnitsTests::format_btc_thinSpaceSeparators()
{
    const QString formatted = QmlBitcoinUnits::format(QmlBitcoinUnits::Unit::BTC, 12345 * COIN);
    QCOMPARE(formatted, QString("12") + QChar(0x2009) + QString("345.00000000"));
}

void QmlBitcoinUnitsTests::format_sat_noDecimals()
{
    QCOMPARE(QmlBitcoinUnits::format(QmlBitcoinUnits::Unit::SAT, 123), QString("123"));
}

#ifdef BITCOINQML_NO_TEST_MAIN
#include <test/qt_test_registry.h>
BITCOINQML_REGISTER_QT_TEST(QmlBitcoinUnitsTests)
#else
QTEST_MAIN(QmlBitcoinUnitsTests)
#endif
#include "test_qmlbitcoinunits.moc"
