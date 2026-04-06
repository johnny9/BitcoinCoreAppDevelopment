// Copyright (c) 2026 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QtTest/QtTest>

#include <qml/models/bitcoinaddress.h>

class BitcoinAddressTests : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void formattedAddress_groupsCharactersInFours();
    void ellipsesAddress_keepsVisiblePrefixAndSuffix();
};

void BitcoinAddressTests::formattedAddress_groupsCharactersInFours()
{
    QCOMPARE(
        BitcoinAddress::formattedAddress("abcd1234efgh5678"),
        QString("abcd 1234 efgh 5678"));
}

void BitcoinAddressTests::ellipsesAddress_keepsVisiblePrefixAndSuffix()
{
    QCOMPARE(
        BitcoinAddress::ellipsesAddress("abcd1234efgh5678ijkl"),
        QString("abcd 1234 ... 5678 ijkl"));
}

int RunBitcoinAddressTests(int argc, char* argv[])
{
    BitcoinAddressTests tests;
    return QTest::qExec(&tests, argc, argv);
}

#ifndef BITCOINQML_NO_TEST_MAIN
QTEST_MAIN(BitcoinAddressTests)
#endif
#include "test_bitcoinaddress.moc"
