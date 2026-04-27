// Copyright (c) 2026 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QtTest/QtTest>

#include <test/mocks/mockwallet.h>
#include <qml/models/bumptransactionmodel.h>

#include <QSignalSpy>

#include <memory>

namespace {
using ::testing::Invoke;
using ::testing::NiceMock;
using ::testing::Return;

const auto TEST_TXID = QStringLiteral("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

std::unique_ptr<NiceMock<MockWallet>> MakeMockWallet()
{
    auto wallet = std::make_unique<NiceMock<MockWallet>>();
    ON_CALL(*wallet, transactionCanBeBumped(testing::_)).WillByDefault(Return(true));
    return wallet;
}
} // namespace

class BumpTransactionModelTests : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void stateIsIdleByDefault()
    {
        auto wallet = MakeMockWallet();
        BumpTransactionModel model(wallet.get());
        QCOMPARE(model.state(), BumpTransactionModel::Idle);
    }

    void prepareFeeBump_transitionsToNeedsConfirmation()
    {
        auto wallet = MakeMockWallet();
        ON_CALL(*wallet, createBumpTransaction(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
            .WillByDefault(Invoke([](const Txid&, const wallet::CCoinControl&, std::vector<bilingual_str>&,
                                     CAmount& old_fee, CAmount& new_fee, CMutableTransaction&) {
                old_fee = 500;
                new_fee = 1000;
                return true;
            }));

        BumpTransactionModel model(wallet.get());
        QSignalSpy stateSpy(&model, &BumpTransactionModel::stateChanged);

        model.prepareFeeBump(TEST_TXID, 1);

        QCOMPARE(model.state(), BumpTransactionModel::NeedsConfirmation);
        QVERIFY(stateSpy.count() >= 2); // Idle -> Preparing -> NeedsConfirmation
    }

    void prepareFeeBump_populatesFees()
    {
        auto wallet = MakeMockWallet();
        ON_CALL(*wallet, createBumpTransaction(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
            .WillByDefault(Invoke([](const Txid&, const wallet::CCoinControl&, std::vector<bilingual_str>&,
                                     CAmount& old_fee, CAmount& new_fee, CMutableTransaction&) {
                old_fee = 500;
                new_fee = 1000;
                return true;
            }));

        BumpTransactionModel model(wallet.get());
        model.prepareFeeBump(TEST_TXID, 1);

        QVERIFY(!model.oldFee().isEmpty());
        QVERIFY(!model.newFee().isEmpty());
        QVERIFY(!model.feeIncrease().isEmpty());
    }

    void prepareFeeBump_failureSetsErrorState()
    {
        auto wallet = MakeMockWallet();
        ON_CALL(*wallet, createBumpTransaction(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
            .WillByDefault(Invoke([](const Txid&, const wallet::CCoinControl&, std::vector<bilingual_str>& errors,
                                     CAmount&, CAmount&, CMutableTransaction&) {
                errors.emplace_back(Untranslated("insufficient fee"));
                return false;
            }));

        BumpTransactionModel model(wallet.get());
        model.prepareFeeBump(TEST_TXID, 1);

        QCOMPARE(model.state(), BumpTransactionModel::Failed);
        QVERIFY(!model.errorText().isEmpty());
    }

    void prepareFeeBump_invalidTxidFails()
    {
        auto wallet = MakeMockWallet();
        BumpTransactionModel model(wallet.get());

        model.prepareFeeBump(QStringLiteral("not-a-txid"), 1);

        QCOMPARE(model.state(), BumpTransactionModel::Failed);
    }

    void prepareFeeBump_nullWalletFails()
    {
        BumpTransactionModel model(nullptr);

        model.prepareFeeBump(TEST_TXID, 1);

        QCOMPARE(model.state(), BumpTransactionModel::Failed);
    }

    void confirmFeeBump_signsAndCommits()
    {
        auto wallet = MakeMockWallet();
        ON_CALL(*wallet, createBumpTransaction(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
            .WillByDefault(Invoke([](const Txid&, const wallet::CCoinControl&, std::vector<bilingual_str>&,
                                     CAmount& old_fee, CAmount& new_fee, CMutableTransaction&) {
                old_fee = 500;
                new_fee = 1000;
                return true;
            }));
        ON_CALL(*wallet, signBumpTransaction(testing::_)).WillByDefault(Return(true));
        ON_CALL(*wallet, commitBumpTransaction(testing::_, testing::_, testing::_, testing::_))
            .WillByDefault(Invoke([](const Txid&, CMutableTransaction&&, std::vector<bilingual_str>&, Txid& bumped_txid) {
                bumped_txid = Txid::FromUint256(*uint256::FromHex("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"));
                return true;
            }));

        BumpTransactionModel model(wallet.get());
        model.prepareFeeBump(TEST_TXID, 1);
        QCOMPARE(model.state(), BumpTransactionModel::NeedsConfirmation);

        model.confirmFeeBump();

        QCOMPARE(model.state(), BumpTransactionModel::Succeeded);
        QVERIFY(!model.newTxid().isEmpty());
    }

    void confirmFeeBump_signFailureDoesNotCommit()
    {
        auto wallet = MakeMockWallet();
        ON_CALL(*wallet, createBumpTransaction(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
            .WillByDefault(Invoke([](const Txid&, const wallet::CCoinControl&, std::vector<bilingual_str>&,
                                     CAmount& old_fee, CAmount& new_fee, CMutableTransaction&) {
                old_fee = 500;
                new_fee = 1000;
                return true;
            }));
        ON_CALL(*wallet, signBumpTransaction(testing::_)).WillByDefault(Return(false));
        EXPECT_CALL(*wallet, commitBumpTransaction(testing::_, testing::_, testing::_, testing::_)).Times(0);

        BumpTransactionModel model(wallet.get());
        model.prepareFeeBump(TEST_TXID, 1);

        model.confirmFeeBump();

        QCOMPARE(model.state(), BumpTransactionModel::Failed);
    }

    void confirmFeeBump_rejectsWhenNotReady()
    {
        auto wallet = MakeMockWallet();
        BumpTransactionModel model(wallet.get());

        model.confirmFeeBump();

        QCOMPARE(model.state(), BumpTransactionModel::Idle);
    }

    void confirmFeeBump_rechecksEligibility()
    {
        auto wallet = MakeMockWallet();
        ON_CALL(*wallet, createBumpTransaction(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
            .WillByDefault(Invoke([](const Txid&, const wallet::CCoinControl&, std::vector<bilingual_str>&,
                                     CAmount& old_fee, CAmount& new_fee, CMutableTransaction&) {
                old_fee = 500;
                new_fee = 1000;
                return true;
            }));

        BumpTransactionModel model(wallet.get());
        model.prepareFeeBump(TEST_TXID, 1);

        ON_CALL(*wallet, transactionCanBeBumped(testing::_)).WillByDefault(Return(false));
        model.confirmFeeBump();

        QCOMPARE(model.state(), BumpTransactionModel::Failed);
    }

    void reset_clearsState()
    {
        auto wallet = MakeMockWallet();
        ON_CALL(*wallet, createBumpTransaction(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
            .WillByDefault(Invoke([](const Txid&, const wallet::CCoinControl&, std::vector<bilingual_str>&,
                                     CAmount& old_fee, CAmount& new_fee, CMutableTransaction&) {
                old_fee = 500;
                new_fee = 1000;
                return true;
            }));

        BumpTransactionModel model(wallet.get());
        model.prepareFeeBump(TEST_TXID, 1);
        QCOMPARE(model.state(), BumpTransactionModel::NeedsConfirmation);

        model.reset();

        QCOMPARE(model.state(), BumpTransactionModel::Idle);
        QVERIFY(model.newTxid().isEmpty());
        QVERIFY(model.errorText().isEmpty());
    }
};

int RunBumpTransactionModelTests(int argc, char* argv[])
{
    BumpTransactionModelTests test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_bumptransactionmodel.moc"
