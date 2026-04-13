// Copyright (c) 2026 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QtTest/QtTest>

#include <qml/bitcoinamount.h>
#include <qml/models/sendrecipient.h>
#include <qml/models/sendrecipientslistmodel.h>
#include <qml/models/walletqmlmodeltransaction.h>

#include <consensus/amount.h>

class WalletQmlModelTransactionTests : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void singleRecipientReviewAmountsInheritRecipientUnit();
    void multipleRecipientReviewAmountsStayInBtc();
    void recipientRolesExposeFormattedAddressAndUnitLabel();
};

void WalletQmlModelTransactionTests::singleRecipientReviewAmountsInheritRecipientUnit()
{
    SendRecipientsListModel recipients;
    auto* recipient = recipients.currentRecipient();
    recipient->amount()->setUnit(BitcoinAmount::Unit::SAT);
    recipient->amount()->setSatoshi(1250);

    WalletQmlModelTransaction transaction(&recipients);
    transaction.setTransactionFee(50);

    QCOMPARE(transaction.feeAmount()->unit(), BitcoinAmount::Unit::SAT);
    QCOMPARE(transaction.totalAmount()->unit(), BitcoinAmount::Unit::SAT);
    QCOMPARE(transaction.feeAmount()->toDisplay(), QString("50"));
    QCOMPARE(transaction.totalAmount()->toDisplay(), QString("1300"));
}

void WalletQmlModelTransactionTests::multipleRecipientReviewAmountsStayInBtc()
{
    SendRecipientsListModel recipients;
    auto* first = recipients.currentRecipient();
    first->amount()->setUnit(BitcoinAmount::Unit::BTC);
    first->amount()->setSatoshi(COIN / 2);

    recipients.add();
    auto* second = recipients.currentRecipient();
    second->amount()->setUnit(BitcoinAmount::Unit::SAT);
    second->amount()->setSatoshi(2000);

    WalletQmlModelTransaction transaction(&recipients);
    transaction.setTransactionFee(1000);

    QCOMPARE(transaction.feeAmount()->unit(), BitcoinAmount::Unit::BTC);
    QCOMPARE(transaction.totalAmount()->unit(), BitcoinAmount::Unit::BTC);
    QCOMPARE(transaction.feeAmount()->toDisplay(), QString("0.00001000"));
    QCOMPARE(transaction.totalAmount()->toDisplay(), QString("0.50003000"));
}

void WalletQmlModelTransactionTests::recipientRolesExposeFormattedAddressAndUnitLabel()
{
    SendRecipientsListModel recipients;
    auto* recipient = recipients.currentRecipient();
    recipient->address()->setAddress("abcd1234efgh5678", 0);
    recipient->amount()->setUnit(BitcoinAmount::Unit::SAT);
    recipient->amount()->setSatoshi(42);

    const QModelIndex index = recipients.index(0, 0);
    QCOMPARE(
        recipients.data(index, SendRecipientsListModel::FormattedAddressRole).toString(),
        QString("abcd 1234 efgh 5678"));
    QCOMPARE(
        recipients.data(index, SendRecipientsListModel::AmountUnitLabelRole).toString(),
        QString("sat"));
}

#ifdef BITCOINQML_NO_TEST_MAIN
#include <test/qt_test_registry.h>
BITCOINQML_REGISTER_QT_TEST(WalletQmlModelTransactionTests)
#else
QTEST_MAIN(WalletQmlModelTransactionTests)
#endif
#include "test_walletqmlmodeltransaction.moc"
