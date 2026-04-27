// Copyright (c) 2026 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QtTest/QtTest>

#include <test/mocks/mockwallet.h>
#include <qml/models/sendrecipient.h>
#include <qml/models/sendrecipientslistmodel.h>
#include <qml/models/walletqmlmodel.h>
#include <qml/models/walletqmlmodeltransaction.h>

#include <chainparams.h>
#include <key_io.h>
#include <primitives/transaction.h>

#include <QSignalSpy>
#include <QSemaphore>

#include <algorithm>
#include <atomic>
#include <memory>
#include <vector>

namespace {
using ::testing::Invoke;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::AtLeast;

constexpr auto FEE_ESTIMATE_TIMEOUT_MS{3'000};
const auto VALID_MAINNET_ADDRESS = QStringLiteral("1BoatSLRHtKNngkdXEeobR76b53LETtpyT");
const auto VALID_REGTEST_ADDRESS = QStringLiteral("bcrt1qdavt4j2sd7dlhqsavtnfxvzppw6k7qy97tmnu9");

class ChainSelectionGuard
{
public:
    explicit ChainSelectionGuard(const ChainType chain_type)
        : m_previous_chain_type{Params().GetChainType()}
    {
        SelectParams(chain_type);
    }

    ~ChainSelectionGuard()
    {
        SelectParams(m_previous_chain_type);
    }

private:
    ChainType m_previous_chain_type;
};

std::unique_ptr<WalletQmlModel> MakeWalletModel(NiceMock<MockWallet>*& wallet_out)
{
    auto wallet = std::make_unique<NiceMock<MockWallet>>();
    wallet_out = wallet.get();

    ON_CALL(*wallet_out, getWalletTxs()).WillByDefault(Return(std::set<interfaces::WalletTx>{}));
    ON_CALL(*wallet_out, listCoins()).WillByDefault(Return(interfaces::Wallet::CoinsList{}));
    ON_CALL(*wallet_out, getBalance()).WillByDefault(Return(10 * COIN));
    ON_CALL(*wallet_out, getRequiredFee(testing::_)).WillByDefault(Return(1000));
    ON_CALL(*wallet_out, getDefaultAddressType()).WillByDefault(Return(OutputType::BECH32));
    ON_CALL(*wallet_out, handleTransactionChanged(testing::_)).WillByDefault(Invoke([](interfaces::Wallet::TransactionChangedFn) {
        return std::unique_ptr<interfaces::Handler>{};
    }));
    ON_CALL(*wallet_out, getNewDestinationValue(testing::_, testing::_)).WillByDefault(Invoke([](OutputType, const std::string&) {
        return DecodeDestination(VALID_MAINNET_ADDRESS.toStdString());
    }));

    return std::make_unique<WalletQmlModel>(std::move(wallet));
}

void SetValidRecipient(WalletQmlModel& model,
                       const QString& address = VALID_MAINNET_ADDRESS,
                       const bool subtract_fee_from_amount = false)
{
    auto* recipient = model.sendRecipientList()->currentRecipient();
    QVERIFY(recipient != nullptr);

    recipient->address()->setAddress(address, 0);
    recipient->amount()->setSatoshi(50'000);
    recipient->setSubtractFeeFromAmount(subtract_fee_from_amount);

    QVERIFY2(recipient->isValid(), "Recipient must be valid before scheduling fee estimates");
}
} // namespace

class WalletQmlModelTests : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void feeTargetIndex_mapsStandardTargets();
    void estimatedFeeForTarget_returnsEmptyWhenUnavailable();
    void scheduleFeeEstimates_populatesFormattedEstimates();
    void scheduleFeeEstimates_fallsBackWhenNetworkFeeEstimatesUnavailable();
    void scheduleFeeEstimates_usesStaticRegtestFeeOverride();
    void scheduleFeeEstimates_usesCustomFeeRateWhenEnabled();
    void prepareTransaction_usesStaticRegtestFeeOverride();
    void prepareTransaction_usesCustomFeeRateWithoutRegtestOverride();
    void prepareTransaction_reassignsAmountWhenFeeIncluded();
    void walletQmlModelTransaction_reassignAmounts_excludesChangeOutput();
    void scheduleFeeEstimates_usesSelectedCoinsInCoinControl();
    void scheduleFeeEstimates_debouncesRapidRestarts();
};

void WalletQmlModelTests::initTestCase()
{
    SelectParams(ChainType::MAIN);
}

void WalletQmlModelTests::feeTargetIndex_mapsStandardTargets()
{
    WalletQmlModel model;

    QCOMPARE(model.feeTargetIndex(1), 0);
    QCOMPARE(model.feeTargetIndex(2), 1);
    QCOMPARE(model.feeTargetIndex(6), 2);
    QCOMPARE(model.feeTargetIndex(42), 1);
}

void WalletQmlModelTests::estimatedFeeForTarget_returnsEmptyWhenUnavailable()
{
    WalletQmlModel model;

    QCOMPARE(model.estimatedFeeForTarget(1), QString());
    QCOMPARE(model.estimatedFeeForTarget(2), QString());
    QCOMPARE(model.estimatedFee(), QString());
}

void WalletQmlModelTests::scheduleFeeEstimates_populatesFormattedEstimates()
{
    NiceMock<MockWallet>* wallet{nullptr};
    auto model = MakeWalletModel(wallet);
    SetValidRecipient(*model);

    QSemaphore release_first_call;
    std::atomic<bool> first_call_started{false};
    std::atomic<bool> first_call_blocked{false};
    std::atomic<bool> saw_sign_true{false};
    std::atomic<bool> saw_wrong_recipient_count{false};
    std::atomic<bool> saw_selected_inputs{false};
    std::atomic<bool> saw_nonempty_feerate{false};
    std::vector<unsigned int> requested_targets;

    EXPECT_CALL(*wallet, getNewDestinationValue(OutputType::BECH32, "qml-fee-preview")).Times(1);
    wallet->createTransactionHandler = [&](const std::vector<wallet::CRecipient>& recipients,
                                           const wallet::CCoinControl& coin_control,
                                           bool sign,
                                           int& change_pos,
                                           CAmount& fee) -> util::Result<CTransactionRef> {
        if (sign) saw_sign_true = true;
        if (recipients.size() != 1U) saw_wrong_recipient_count = true;
        if (coin_control.HasSelected() || !coin_control.ListSelected().empty()) saw_selected_inputs = true;
        if (coin_control.m_feerate.has_value()) saw_nonempty_feerate = true;

        requested_targets.push_back(coin_control.m_confirm_target.value_or(0));
        change_pos = -1;
        fee = coin_control.m_confirm_target.value_or(0) * 100;
        if (!first_call_blocked.exchange(true)) {
            first_call_started = true;
            release_first_call.acquire();
        }
        return util::Result<CTransactionRef>{MakeTransactionRef(CMutableTransaction{})};
    };

    model->scheduleFeeEstimates();

    QTRY_VERIFY_WITH_TIMEOUT(first_call_started.load(), FEE_ESTIMATE_TIMEOUT_MS);
    QTRY_VERIFY_WITH_TIMEOUT(model->feeEstimatePending(), FEE_ESTIMATE_TIMEOUT_MS);
    QCOMPARE(model->estimatedFeeForTarget(2), QString());

    release_first_call.release();

    QTRY_VERIFY_WITH_TIMEOUT(requested_targets.size() == 3, FEE_ESTIMATE_TIMEOUT_MS);
    QVERIFY(!saw_sign_true.load());
    QVERIFY(!saw_wrong_recipient_count.load());
    QVERIFY(!saw_selected_inputs.load());
    QVERIFY(!saw_nonempty_feerate.load());
    QCOMPARE(requested_targets.at(0), 1U);
    QCOMPARE(requested_targets.at(1), 2U);
    QCOMPARE(requested_targets.at(2), 6U);
    QTRY_VERIFY_WITH_TIMEOUT(!model->feeEstimatePending(), FEE_ESTIMATE_TIMEOUT_MS);
    QCOMPARE(model->estimatedFeeForTarget(1), QStringLiteral("0.00000100 ₿"));
    QCOMPARE(model->estimatedFeeForTarget(2), QStringLiteral("0.00000200 ₿"));
    QCOMPARE(model->estimatedFeeForTarget(6), QStringLiteral("0.00000600 ₿"));
    QCOMPARE(model->estimatedFee(), QStringLiteral("0.00000200 ₿"));
}

void WalletQmlModelTests::scheduleFeeEstimates_fallsBackWhenNetworkFeeEstimatesUnavailable()
{
    NiceMock<MockWallet>* wallet{nullptr};
    auto model = MakeWalletModel(wallet);
    SetValidRecipient(*model);

    std::atomic<bool> saw_fallback_feerate{false};
    std::vector<CAmount> fallback_fee_rates;

    EXPECT_CALL(*wallet, getNewDestinationValue(OutputType::BECH32, "qml-fee-preview")).Times(1);
    EXPECT_CALL(*wallet, getRequiredFee(1000)).Times(AtLeast(3)).WillRepeatedly(Return(1000));
    wallet->createTransactionHandler = [&](const std::vector<wallet::CRecipient>&,
                                           const wallet::CCoinControl& coin_control,
                                           bool,
                                           int& change_pos,
                                           CAmount& fee) -> util::Result<CTransactionRef> {
        change_pos = -1;

        if (!coin_control.m_feerate.has_value()) {
            return util::Error{Untranslated("fee estimation unavailable")};
        }

        saw_fallback_feerate = true;
        fallback_fee_rates.push_back(coin_control.m_feerate->GetFeePerK());
        fee = coin_control.m_feerate->GetFee(250);
        return util::Result<CTransactionRef>{MakeTransactionRef(CMutableTransaction{})};
    };

    model->scheduleFeeEstimates();

    QTRY_COMPARE_WITH_TIMEOUT(model->estimatedFeeForTarget(2), QStringLiteral("0.00000500 ₿"), FEE_ESTIMATE_TIMEOUT_MS);
    QTRY_VERIFY_WITH_TIMEOUT(!model->feeEstimatePending(), FEE_ESTIMATE_TIMEOUT_MS);
    QVERIFY(saw_fallback_feerate.load());
    QVERIFY(!fallback_fee_rates.empty());
    QCOMPARE(model->estimatedFeeForTarget(1), QStringLiteral("0.00000750 ₿"));
    QCOMPARE(model->estimatedFeeForTarget(2), QStringLiteral("0.00000500 ₿"));
    QCOMPARE(model->estimatedFeeForTarget(6), QStringLiteral("0.00000250 ₿"));
    QCOMPARE(model->estimatedFee(), QStringLiteral("0.00000500 ₿"));
    QCOMPARE(fallback_fee_rates.size(), 3U);
    QCOMPARE(fallback_fee_rates.at(0), CAmount{3000});
    QCOMPARE(fallback_fee_rates.at(1), CAmount{2000});
    QCOMPARE(fallback_fee_rates.at(2), CAmount{1000});
}

void WalletQmlModelTests::scheduleFeeEstimates_usesStaticRegtestFeeOverride()
{
    ChainSelectionGuard chain_guard{ChainType::REGTEST};
    NiceMock<MockWallet>* wallet{nullptr};
    auto model = MakeWalletModel(wallet);
    SetValidRecipient(*model, VALID_REGTEST_ADDRESS);

    bool saw_sign_true{false};
    std::vector<unsigned int> requested_targets;
    std::vector<CAmount> requested_fee_rates;

    EXPECT_CALL(*wallet, getNewDestinationValue(OutputType::BECH32, "qml-fee-preview")).Times(1);
    EXPECT_CALL(*wallet, getRequiredFee(1000)).Times(0);
    wallet->createTransactionHandler = [&](const std::vector<wallet::CRecipient>&,
                                           const wallet::CCoinControl& coin_control,
                                           bool sign,
                                           int& change_pos,
                                           CAmount& fee) -> util::Result<CTransactionRef> {
        if (sign) saw_sign_true = true;
        requested_targets.push_back(coin_control.m_confirm_target.value_or(0));
        requested_fee_rates.push_back(coin_control.m_feerate.has_value() ? coin_control.m_feerate->GetFeePerK() : 0);
        change_pos = -1;

        if (!coin_control.m_feerate.has_value()) return util::Error{Untranslated("missing regtest fee override")};

        fee = coin_control.m_feerate->GetFee(250);
        return util::Result<CTransactionRef>{MakeTransactionRef(CMutableTransaction{})};
    };

    model->scheduleFeeEstimates();

    QTRY_COMPARE_WITH_TIMEOUT(model->estimatedFeeForTarget(2), QStringLiteral("0.00000250 ₿"), FEE_ESTIMATE_TIMEOUT_MS);
    QVERIFY(!saw_sign_true);
    QCOMPARE(model->estimatedFeeForTarget(1), QStringLiteral("0.00000250 ₿"));
    QCOMPARE(model->estimatedFeeForTarget(2), QStringLiteral("0.00000250 ₿"));
    QCOMPARE(model->estimatedFeeForTarget(6), QStringLiteral("0.00000250 ₿"));
    QCOMPARE(requested_targets.size(), 3U);
    QCOMPARE(requested_targets.at(0), 0U);
    QCOMPARE(requested_targets.at(1), 0U);
    QCOMPARE(requested_targets.at(2), 0U);
    QCOMPARE(requested_fee_rates.size(), 3U);
    QCOMPARE(requested_fee_rates.at(0), CAmount{wallet::DEFAULT_TRANSACTION_MINFEE});
    QCOMPARE(requested_fee_rates.at(1), CAmount{wallet::DEFAULT_TRANSACTION_MINFEE});
    QCOMPARE(requested_fee_rates.at(2), CAmount{wallet::DEFAULT_TRANSACTION_MINFEE});
}

void WalletQmlModelTests::scheduleFeeEstimates_usesCustomFeeRateWhenEnabled()
{
    NiceMock<MockWallet>* wallet{nullptr};
    auto model = MakeWalletModel(wallet);
    SetValidRecipient(*model);

    std::vector<unsigned int> requested_targets;
    std::vector<CAmount> requested_fee_rates;

    EXPECT_CALL(*wallet, getNewDestinationValue(OutputType::BECH32, "qml-fee-preview")).Times(1);
    wallet->createTransactionHandler = [&](const std::vector<wallet::CRecipient>&,
                                           const wallet::CCoinControl& coin_control,
                                           bool,
                                           int& change_pos,
                                           CAmount& fee) -> util::Result<CTransactionRef> {
        requested_targets.push_back(coin_control.m_confirm_target.value_or(0));
        requested_fee_rates.push_back(coin_control.m_feerate.has_value() ? coin_control.m_feerate->GetFeePerK() : 0);
        change_pos = -1;
        fee = coin_control.m_feerate.has_value()
            ? coin_control.m_feerate->GetFee(250)
            : coin_control.m_confirm_target.value_or(0) * 100;
        return util::Result<CTransactionRef>{MakeTransactionRef(CMutableTransaction{})};
    };

    model->setCustomFeeEnabled(true);
    model->setCustomFeeRate(QStringLiteral("2"));
    model->scheduleFeeEstimates();

    QTRY_COMPARE_WITH_TIMEOUT(model->estimatedFee(), QStringLiteral("0.00000500 ₿"), FEE_ESTIMATE_TIMEOUT_MS);
    QCOMPARE(requested_targets.size(), 4U);
    QCOMPARE(requested_fee_rates.size(), 4U);
    QVERIFY(std::find(requested_fee_rates.begin(), requested_fee_rates.end(), CAmount{2000}) != requested_fee_rates.end());
    QVERIFY(std::find(requested_targets.begin(), requested_targets.end(), 1U) != requested_targets.end());
    QVERIFY(std::find(requested_targets.begin(), requested_targets.end(), 2U) != requested_targets.end());
    QVERIFY(std::find(requested_targets.begin(), requested_targets.end(), 6U) != requested_targets.end());
    QVERIFY(std::find(requested_targets.begin(), requested_targets.end(), 0U) != requested_targets.end());
}

void WalletQmlModelTests::prepareTransaction_usesStaticRegtestFeeOverride()
{
    ChainSelectionGuard chain_guard{ChainType::REGTEST};
    NiceMock<MockWallet>* wallet{nullptr};
    auto model = MakeWalletModel(wallet);
    SetValidRecipient(*model, VALID_REGTEST_ADDRESS);

    bool saw_sign_false{false};
    std::vector<unsigned int> requested_targets;
    std::vector<CAmount> requested_fee_rates;

    EXPECT_CALL(*wallet, getRequiredFee(1000)).Times(0);
    wallet->createTransactionHandler = [&](const std::vector<wallet::CRecipient>&,
                                           const wallet::CCoinControl& coin_control,
                                           bool sign,
                                           int& change_pos,
                                           CAmount& fee) -> util::Result<CTransactionRef> {
        if (!sign) saw_sign_false = true;
        requested_targets.push_back(coin_control.m_confirm_target.value_or(0));
        requested_fee_rates.push_back(coin_control.m_feerate.has_value() ? coin_control.m_feerate->GetFeePerK() : 0);
        change_pos = -1;

        if (!coin_control.m_feerate.has_value()) return util::Error{Untranslated("missing regtest fee override")};

        fee = coin_control.m_feerate->GetFee(250);
        return util::Result<CTransactionRef>{MakeTransactionRef(CMutableTransaction{})};
    };

    QVERIFY(model->prepareTransaction());
    QVERIFY(!saw_sign_false);
    QVERIFY(model->currentTransaction() != nullptr);
    QCOMPARE(model->currentTransaction()->getTransactionFee(), CAmount{250});
    QCOMPARE(requested_targets.size(), 1U);
    QCOMPARE(requested_targets.at(0), 0U);
    QCOMPARE(requested_fee_rates.size(), 1U);
    QCOMPARE(requested_fee_rates.at(0), CAmount{wallet::DEFAULT_TRANSACTION_MINFEE});
}

void WalletQmlModelTests::prepareTransaction_usesCustomFeeRateWithoutRegtestOverride()
{
    ChainSelectionGuard chain_guard{ChainType::REGTEST};
    NiceMock<MockWallet>* wallet{nullptr};
    auto model = MakeWalletModel(wallet);
    SetValidRecipient(*model, VALID_REGTEST_ADDRESS);

    std::vector<unsigned int> requested_targets;
    std::vector<CAmount> requested_fee_rates;

    EXPECT_CALL(*wallet, getRequiredFee(1000)).Times(0);
    wallet->createTransactionHandler = [&](const std::vector<wallet::CRecipient>&,
                                           const wallet::CCoinControl& coin_control,
                                           bool,
                                           int& change_pos,
                                           CAmount& fee) -> util::Result<CTransactionRef> {
        requested_targets.push_back(coin_control.m_confirm_target.value_or(0));
        requested_fee_rates.push_back(coin_control.m_feerate.has_value() ? coin_control.m_feerate->GetFeePerK() : 0);
        change_pos = -1;

        if (!coin_control.m_feerate.has_value()) {
            return util::Error{Untranslated("missing custom fee override")};
        }

        fee = coin_control.m_feerate->GetFee(250);
        return util::Result<CTransactionRef>{MakeTransactionRef(CMutableTransaction{})};
    };

    model->setCustomFeeEnabled(true);
    model->setCustomFeeRate(QStringLiteral("2"));

    QVERIFY(model->prepareTransaction());
    QVERIFY(model->currentTransaction() != nullptr);
    QCOMPARE(model->currentTransaction()->getTransactionFee(), CAmount{500});
    QCOMPARE(requested_targets.size(), 1U);
    QCOMPARE(requested_targets.at(0), 0U);
    QCOMPARE(requested_fee_rates.size(), 1U);
    QCOMPARE(requested_fee_rates.at(0), CAmount{2000});
}

void WalletQmlModelTests::prepareTransaction_reassignsAmountWhenFeeIncluded()
{
    NiceMock<MockWallet>* wallet{nullptr};
    auto model = MakeWalletModel(wallet);
    SetValidRecipient(*model, VALID_MAINNET_ADDRESS, /*subtract_fee_from_amount=*/true);

    bool saw_subtract_fee_from_amount{false};
    bool saw_wrong_recipient_count{false};
    wallet->createTransactionHandler = [&](const std::vector<wallet::CRecipient>& recipients,
                                           const wallet::CCoinControl&,
                                           bool,
                                           int& change_pos,
                                           CAmount& fee) -> util::Result<CTransactionRef> {
        if (recipients.size() != 1U) {
            saw_wrong_recipient_count = true;
            return util::Error{Untranslated("unexpected recipient count")};
        }
        saw_subtract_fee_from_amount = recipients.at(0).fSubtractFeeFromAmount;
        change_pos = -1;
        fee = 200;

        CMutableTransaction tx;
        tx.vout.emplace_back(/*nValue=*/49'800, CScript{});
        return util::Result<CTransactionRef>{MakeTransactionRef(std::move(tx))};
    };

    QVERIFY(model->prepareTransaction());
    QVERIFY(!saw_wrong_recipient_count);
    QVERIFY(saw_subtract_fee_from_amount);
    QVERIFY(model->currentTransaction() != nullptr);
    QCOMPARE(model->currentTransaction()->amount(), QStringLiteral("49800"));
    QCOMPARE(model->currentTransaction()->fee(), QStringLiteral("200"));
    QCOMPARE(model->currentTransaction()->total(), QStringLiteral("50000"));
    QCOMPARE(model->currentTransaction()->getTotalTransactionAmount(), CAmount{50'000});
}

void WalletQmlModelTests::walletQmlModelTransaction_reassignAmounts_excludesChangeOutput()
{
    SendRecipientsListModel recipients;
    auto* recipient = recipients.currentRecipient();
    QVERIFY(recipient != nullptr);

    recipient->address()->setAddress(VALID_MAINNET_ADDRESS, 0);
    recipient->amount()->setSatoshi(50'000);

    WalletQmlModelTransaction transaction{&recipients};
    QSignalSpy amount_changed_spy{&transaction, &WalletQmlModelTransaction::amountChanged};
    QSignalSpy total_changed_spy{&transaction, &WalletQmlModelTransaction::totalChanged};
    QSignalSpy fee_changed_spy{&transaction, &WalletQmlModelTransaction::feeChanged};

    CMutableTransaction tx;
    tx.vout.emplace_back(/*nValue=*/49'800, CScript{});
    tx.vout.emplace_back(/*nValue=*/1'000, CScript{}); // change
    transaction.setWtx(MakeTransactionRef(std::move(tx)));

    transaction.setTransactionFee(200);
    QCOMPARE(fee_changed_spy.count(), 1);
    QCOMPARE(total_changed_spy.count(), 1);
    QCOMPARE(transaction.total(), QStringLiteral("50200"));

    transaction.reassignAmounts(/*nChangePosRet=*/1);

    QCOMPARE(amount_changed_spy.count(), 1);
    QCOMPARE(total_changed_spy.count(), 2);
    QCOMPARE(transaction.amount(), QStringLiteral("49800"));
    QCOMPARE(transaction.fee(), QStringLiteral("200"));
    QCOMPARE(transaction.total(), QStringLiteral("50000"));
    QCOMPARE(transaction.getTotalTransactionAmount(), CAmount{50'000});
}

void WalletQmlModelTests::scheduleFeeEstimates_usesSelectedCoinsInCoinControl()
{
    NiceMock<MockWallet>* wallet{nullptr};
    auto model = MakeWalletModel(wallet);
    SetValidRecipient(*model);

    const COutPoint selected_outpoint{Txid{}, 5};
    std::atomic<int> create_transaction_calls{0};
    std::atomic<bool> saw_sign_true{false};
    std::atomic<bool> saw_missing_selection{false};
    std::atomic<int> selected_count{-1};
    std::vector<unsigned int> selected_targets;

    EXPECT_CALL(*wallet, getNewDestinationValue(OutputType::BECH32, "qml-fee-preview")).Times(1);
    wallet->createTransactionHandler = [&](const std::vector<wallet::CRecipient>&,
                                           const wallet::CCoinControl& coin_control,
                                           bool sign,
                                           int& change_pos,
                                           CAmount& fee) -> util::Result<CTransactionRef> {
        if (sign) saw_sign_true = true;
        if (!coin_control.HasSelected() || !coin_control.IsSelected(selected_outpoint)) saw_missing_selection = true;
        const auto selected = coin_control.ListSelected();
        selected_count = static_cast<int>(selected.size());
        if (selected.size() != 1U || selected.front() != selected_outpoint) saw_missing_selection = true;

        selected_targets.push_back(coin_control.m_confirm_target.value_or(0));
        ++create_transaction_calls;
        change_pos = -1;
        fee = coin_control.m_confirm_target.value_or(0) * 200;
        return util::Result<CTransactionRef>{MakeTransactionRef(CMutableTransaction{})};
    };

    model->selectCoin(selected_outpoint);

    QTRY_COMPARE_WITH_TIMEOUT(create_transaction_calls.load(), 3, FEE_ESTIMATE_TIMEOUT_MS);
    QVERIFY(!saw_sign_true.load());
    QVERIFY(!saw_missing_selection.load());
    QCOMPARE(selected_count.load(), 1);
    QCOMPARE(selected_targets.size(), 3U);
    QCOMPARE(selected_targets.at(0), 1U);
    QCOMPARE(selected_targets.at(1), 2U);
    QCOMPARE(selected_targets.at(2), 6U);
}

void WalletQmlModelTests::scheduleFeeEstimates_debouncesRapidRestarts()
{
    NiceMock<MockWallet>* wallet{nullptr};
    auto model = MakeWalletModel(wallet);
    SetValidRecipient(*model);

    std::atomic<int> create_transaction_calls{0};

    EXPECT_CALL(*wallet, getNewDestinationValue(OutputType::BECH32, "qml-fee-preview")).Times(1);
    std::atomic<bool> saw_sign_true{false};
    wallet->createTransactionHandler = [&](const std::vector<wallet::CRecipient>&,
                                           const wallet::CCoinControl& coin_control,
                                           bool sign,
                                           int& change_pos,
                                           CAmount& fee) -> util::Result<CTransactionRef> {
        if (sign) saw_sign_true = true;
        ++create_transaction_calls;
        change_pos = -1;
        fee = coin_control.m_confirm_target.value_or(0) * 250;
        return util::Result<CTransactionRef>{MakeTransactionRef(CMutableTransaction{})};
    };

    model->scheduleFeeEstimates();
    model->scheduleFeeEstimates();
    model->scheduleFeeEstimates();

    QTRY_COMPARE_WITH_TIMEOUT(create_transaction_calls.load(), 3, FEE_ESTIMATE_TIMEOUT_MS);
    QVERIFY(!saw_sign_true.load());
    QTRY_VERIFY_WITH_TIMEOUT(!model->feeEstimatePending(), FEE_ESTIMATE_TIMEOUT_MS);
    QCOMPARE(model->estimatedFeeForTarget(1), QStringLiteral("0.00000250 ₿"));
    QCOMPARE(model->estimatedFeeForTarget(2), QStringLiteral("0.00000500 ₿"));
    QCOMPARE(model->estimatedFeeForTarget(6), QStringLiteral("0.00001500 ₿"));
}

int RunWalletQmlModelTests(int argc, char* argv[])
{
    WalletQmlModelTests tests;
    return QTest::qExec(&tests, argc, argv);
}

#ifndef BITCOINQML_NO_TEST_MAIN
QTEST_MAIN(WalletQmlModelTests)
#endif
#include "test_walletqmlmodel.moc"
