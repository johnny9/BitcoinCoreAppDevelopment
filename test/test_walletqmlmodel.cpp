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
#include <common/messages.h>
#include <key_io.h>
#include <primitives/transaction.h>

#include <QSignalSpy>
#include <QSemaphore>

#include <algorithm>
#include <atomic>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include <QSettings>

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

util::Result<wallet::CreatedTransactionResult> MakeCreatedTransactionResult(
    CAmount fee,
    std::optional<unsigned int> change_pos = std::nullopt,
    CMutableTransaction tx = CMutableTransaction{})
{
    return util::Result<wallet::CreatedTransactionResult>{
        wallet::CreatedTransactionResult{MakeTransactionRef(std::move(tx)), fee, change_pos, FeeCalculation{}}};
}

class FakeSecurityWallet : public StubWallet
{
public:
    bool encrypted{true};
    bool locked{true};
    bool private_keys_disabled{false};
    bool external_signer{false};
    CAmount balance{50'000};
    int encrypt_calls{0};
    int change_passphrase_calls{0};
    int backup_calls{0};
    std::string last_backup_path;
    std::vector<std::pair<std::string, std::string>> changed_passphrases;
    int unlock_calls{0};
    int lock_calls{0};
    int commit_calls{0};
    std::vector<std::string> unlock_passphrases;
    std::vector<bool> create_transaction_sign_args;
    std::vector<bool> fill_psbt_sign_args;

    std::function<util::Result<wallet::CreatedTransactionResult>(const std::vector<wallet::CRecipient>&,
                                                                 const wallet::CCoinControl&,
                                                                 bool,
                                                                 std::optional<unsigned int>)>
        create_transaction_fn = [](const std::vector<wallet::CRecipient>&,
                                   const wallet::CCoinControl&,
                                   bool,
                                   std::optional<unsigned int>) {
            CMutableTransaction tx;
            tx.vout.emplace_back(1'000, CScript{});
            return MakeCreatedTransactionResult(250, std::nullopt, std::move(tx));
        };
    std::function<bool(const SecureString&)> unlock_fn = [this](const SecureString& passphrase) {
        ++unlock_calls;
        unlock_passphrases.emplace_back(passphrase.begin(), passphrase.end());
        locked = false;
        return true;
    };
    std::function<std::optional<common::PSBTError>(std::optional<int>,
                                                   bool,
                                                   bool,
                                                   size_t*,
                                                   PartiallySignedTransaction&,
                                                   bool&)>
        fill_psbt_fn = [this](std::optional<int>,
                              bool sign,
                              bool,
                              size_t*,
                              PartiallySignedTransaction&,
                              bool& complete) {
            fill_psbt_sign_args.push_back(sign);
            complete = sign;
            return std::nullopt;
    };

    bool encryptWallet(const SecureString& passphrase) override
    {
        ++encrypt_calls;
        encrypted = true;
        locked = true;
        unlock_passphrases.emplace_back(passphrase.begin(), passphrase.end());
        return !passphrase.empty();
    }
    bool isCrypted() override { return encrypted; }
    bool lock() override
    {
        ++lock_calls;
        locked = true;
        return true;
    }
    bool unlock(const SecureString& wallet_passphrase) override { return unlock_fn(wallet_passphrase); }
    bool isLocked() override { return locked; }
    bool changeWalletPassphrase(const SecureString& old_passphrase, const SecureString& new_passphrase) override
    {
        ++change_passphrase_calls;
        changed_passphrases.emplace_back(
            std::string(old_passphrase.begin(), old_passphrase.end()),
            std::string(new_passphrase.begin(), new_passphrase.end()));
        return !old_passphrase.empty() && !new_passphrase.empty() && old_passphrase == SecureString{"secret"};
    }
    void abortRescan() override {}
    bool backupWallet(const std::string& path) override
    {
        ++backup_calls;
        last_backup_path = path;
        return !path.empty();
    }
    std::string getWalletName() override { return "fake-wallet"; }
    util::Result<wallet::CreatedTransactionResult> createTransaction(const std::vector<wallet::CRecipient>& recipients,
                                                                     const wallet::CCoinControl& coin_control,
                                                                     bool sign,
                                                                     std::optional<unsigned int> change_pos) override
    {
        create_transaction_sign_args.push_back(sign);
        return create_transaction_fn(recipients, coin_control, sign, change_pos);
    }
    void commitTransaction(CTransactionRef, interfaces::WalletValueMap, interfaces::WalletOrderForm) override
    {
        ++commit_calls;
    }
    std::optional<common::PSBTError> fillPSBT(std::optional<int> sighash_type,
                                              bool sign,
                                              bool bip32derivs,
                                              size_t* n_signed,
                                              PartiallySignedTransaction& psbtx,
                                              bool& complete) override
    {
        return fill_psbt_fn(sighash_type, sign, bip32derivs, n_signed, psbtx, complete);
    }
    CAmount getBalance() override { return balance; }
    CAmount getAvailableBalance(const wallet::CCoinControl&) override { return balance; }
    bool privateKeysDisabled() override { return private_keys_disabled; }
    bool hasExternalSigner() override { return external_signer; }
};

std::unique_ptr<WalletQmlModel> MakeSecurityWalletModel(FakeSecurityWallet*& wallet_out)
{
    auto wallet = std::make_unique<FakeSecurityWallet>();
    wallet_out = wallet.get();
    return std::make_unique<WalletQmlModel>(std::move(wallet));
}

void ConfigureSecurityRecipient(WalletQmlModel& model, qint64 satoshis)
{
    auto* recipient = model.sendRecipientList()->currentRecipient();
    QVERIFY(recipient != nullptr);
    recipient->address()->setAddress(VALID_REGTEST_ADDRESS, 0);
    recipient->amount()->setSatoshi(satoshis);
    QVERIFY2(recipient->isValid(), "Recipient must be valid before preparing transaction");
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
    void transactionChangedEmitsBalanceChanged();
    void displayNameDefaultsToWalletName();
    void detailPropertiesReflectWalletCapabilities();
    void encryptWalletUpdatesSecurityState();
    void changeWalletPassphraseForwardsPasswords();
    void backupWalletForwardsPath();
    void prepareTransactionOnLockedWalletMarksUnlockNeeded();
    void sendTransactionOnLockedWalletRequiresPassword();
    void sendTransactionWithPassphraseUnlocksCommitsAndRelocks();
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
                                           std::optional<unsigned int>) -> util::Result<wallet::CreatedTransactionResult> {
        if (sign) saw_sign_true = true;
        if (recipients.size() != 1U) saw_wrong_recipient_count = true;
        if (coin_control.HasSelected() || !coin_control.ListSelected().empty()) saw_selected_inputs = true;
        if (coin_control.m_feerate.has_value()) saw_nonempty_feerate = true;

        requested_targets.push_back(coin_control.m_confirm_target.value_or(0));
        const CAmount fee{coin_control.m_confirm_target.value_or(0) * 100};
        if (!first_call_blocked.exchange(true)) {
            first_call_started = true;
            release_first_call.acquire();
        }
        return MakeCreatedTransactionResult(fee);
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
                                           std::optional<unsigned int>) -> util::Result<wallet::CreatedTransactionResult> {
        if (!coin_control.m_feerate.has_value()) {
            return util::Error{Untranslated("fee estimation unavailable")};
        }

        saw_fallback_feerate = true;
        fallback_fee_rates.push_back(coin_control.m_feerate->GetFeePerK());
        return MakeCreatedTransactionResult(coin_control.m_feerate->GetFee(250));
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
                                           std::optional<unsigned int>) -> util::Result<wallet::CreatedTransactionResult> {
        if (sign) saw_sign_true = true;
        requested_targets.push_back(coin_control.m_confirm_target.value_or(0));
        requested_fee_rates.push_back(coin_control.m_feerate.has_value() ? coin_control.m_feerate->GetFeePerK() : 0);

        if (!coin_control.m_feerate.has_value()) return util::Error{Untranslated("missing regtest fee override")};

        return MakeCreatedTransactionResult(coin_control.m_feerate->GetFee(250));
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
                                           std::optional<unsigned int>) -> util::Result<wallet::CreatedTransactionResult> {
        requested_targets.push_back(coin_control.m_confirm_target.value_or(0));
        requested_fee_rates.push_back(coin_control.m_feerate.has_value() ? coin_control.m_feerate->GetFeePerK() : 0);
        const CAmount fee = coin_control.m_feerate.has_value()
            ? coin_control.m_feerate->GetFee(250)
            : coin_control.m_confirm_target.value_or(0) * 100;
        return MakeCreatedTransactionResult(fee);
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
                                           std::optional<unsigned int>) -> util::Result<wallet::CreatedTransactionResult> {
        if (!sign) saw_sign_false = true;
        requested_targets.push_back(coin_control.m_confirm_target.value_or(0));
        requested_fee_rates.push_back(coin_control.m_feerate.has_value() ? coin_control.m_feerate->GetFeePerK() : 0);

        if (!coin_control.m_feerate.has_value()) return util::Error{Untranslated("missing regtest fee override")};

        return MakeCreatedTransactionResult(coin_control.m_feerate->GetFee(250));
    };

    QVERIFY(model->prepareTransaction());
    QVERIFY(saw_sign_false);
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
                                           std::optional<unsigned int>) -> util::Result<wallet::CreatedTransactionResult> {
        requested_targets.push_back(coin_control.m_confirm_target.value_or(0));
        requested_fee_rates.push_back(coin_control.m_feerate.has_value() ? coin_control.m_feerate->GetFeePerK() : 0);

        if (!coin_control.m_feerate.has_value()) {
            return util::Error{Untranslated("missing custom fee override")};
        }

        return MakeCreatedTransactionResult(coin_control.m_feerate->GetFee(250));
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
                                           std::optional<unsigned int>) -> util::Result<wallet::CreatedTransactionResult> {
        if (recipients.size() != 1U) {
            saw_wrong_recipient_count = true;
            return util::Error{Untranslated("unexpected recipient count")};
        }
        saw_subtract_fee_from_amount = recipients.at(0).fSubtractFeeFromAmount;

        CMutableTransaction tx;
        tx.vout.emplace_back(/*nValue=*/49'800, CScript{});
        return MakeCreatedTransactionResult(/*fee=*/200, std::nullopt, std::move(tx));
    };

    QVERIFY(model->prepareTransaction());
    QVERIFY(!saw_wrong_recipient_count);
    QVERIFY(saw_subtract_fee_from_amount);
    QVERIFY(model->currentTransaction() != nullptr);
    QCOMPARE(model->currentTransaction()->amount(), QStringLiteral("0.00049800 ₿"));
    QCOMPARE(model->currentTransaction()->fee(), QStringLiteral("0.00000200 ₿"));
    QCOMPARE(model->currentTransaction()->total(), QStringLiteral("0.00050000 ₿"));
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
    QCOMPARE(transaction.total(), QStringLiteral("0.00050200 ₿"));

    transaction.reassignAmounts(/*nChangePosRet=*/1);

    QCOMPARE(amount_changed_spy.count(), 1);
    QCOMPARE(total_changed_spy.count(), 2);
    QCOMPARE(transaction.amount(), QStringLiteral("0.00049800 ₿"));
    QCOMPARE(transaction.fee(), QStringLiteral("0.00000200 ₿"));
    QCOMPARE(transaction.total(), QStringLiteral("0.00050000 ₿"));
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
                                           std::optional<unsigned int>) -> util::Result<wallet::CreatedTransactionResult> {
        if (sign) saw_sign_true = true;
        if (!coin_control.HasSelected() || !coin_control.IsSelected(selected_outpoint)) saw_missing_selection = true;
        const auto selected = coin_control.ListSelected();
        selected_count = static_cast<int>(selected.size());
        if (selected.size() != 1U || selected.front() != selected_outpoint) saw_missing_selection = true;

        selected_targets.push_back(coin_control.m_confirm_target.value_or(0));
        ++create_transaction_calls;
        return MakeCreatedTransactionResult(coin_control.m_confirm_target.value_or(0) * 200);
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
                                           std::optional<unsigned int>) -> util::Result<wallet::CreatedTransactionResult> {
        if (sign) saw_sign_true = true;
        ++create_transaction_calls;
        return MakeCreatedTransactionResult(coin_control.m_confirm_target.value_or(0) * 250);
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

void WalletQmlModelTests::transactionChangedEmitsBalanceChanged()
{
    auto wallet = std::make_unique<NiceMock<MockWallet>>();
    auto* wallet_ptr = wallet.get();
    CAmount balance{50 * COIN};
    interfaces::Wallet::TransactionChangedFn transaction_changed;

    ON_CALL(*wallet_ptr, getWalletTxs()).WillByDefault(Return(std::set<interfaces::WalletTx>{}));
    ON_CALL(*wallet_ptr, listCoins()).WillByDefault(Return(interfaces::Wallet::CoinsList{}));
    ON_CALL(*wallet_ptr, getBalance()).WillByDefault(Invoke([&balance] { return balance; }));
    ON_CALL(*wallet_ptr, getRequiredFee(testing::_)).WillByDefault(Return(1000));
    ON_CALL(*wallet_ptr, getDefaultAddressType()).WillByDefault(Return(OutputType::BECH32));
    ON_CALL(*wallet_ptr, handleTransactionChanged(testing::_)).WillByDefault(Invoke([&transaction_changed](interfaces::Wallet::TransactionChangedFn fn) {
        transaction_changed = std::move(fn);
        return std::unique_ptr<interfaces::Handler>{};
    }));

    WalletQmlModel model(std::move(wallet));
    QSignalSpy balance_spy(&model, &WalletQmlModel::balanceChanged);

    QCOMPARE(model.balance(), QStringLiteral("50.00000000"));
    QVERIFY(transaction_changed);

    balance = 75 * COIN;
    transaction_changed(Txid{}, CT_UPDATED);

    QTRY_COMPARE(balance_spy.count(), 1);
    QCOMPARE(model.balance(), QStringLiteral("75.00000000"));
}

void WalletQmlModelTests::displayNameDefaultsToWalletName()
{
    FakeSecurityWallet* wallet{nullptr};
    auto model = MakeSecurityWalletModel(wallet);

    QCOMPARE(model->displayName(), QString("fake-wallet"));
    model->setDisplayName("Personal");
    QCOMPARE(model->displayName(), QString("Personal"));
}

void WalletQmlModelTests::detailPropertiesReflectWalletCapabilities()
{
    FakeSecurityWallet* wallet{nullptr};
    auto model = MakeSecurityWalletModel(wallet);

    wallet->private_keys_disabled = true;
    wallet->external_signer = true;
    QCOMPARE(model->keyScheme(), QString("Watch-only"));
    QCOMPARE(model->privateKeysStatus(), QString("Disabled"));
    QCOMPARE(model->externalSignerStatus(), QString("Enabled"));

    wallet->private_keys_disabled = false;

    QCOMPARE(model->keyScheme(), QString("Single-key"));
    QCOMPARE(model->privateKeysStatus(), QString("Enabled"));
}

void WalletQmlModelTests::encryptWalletUpdatesSecurityState()
{
    auto wallet = std::make_unique<FakeSecurityWallet>();
    wallet->encrypted = false;
    wallet->locked = false;
    FakeSecurityWallet* raw_wallet = wallet.get();
    auto model = std::make_unique<WalletQmlModel>(std::move(wallet));

    QVERIFY(model->encryptWallet("secret"));
    QCOMPARE(raw_wallet->encrypt_calls, 1);
    QVERIFY(model->isEncrypted());
    QVERIFY(model->isLocked());
    QVERIFY(model->settingsError().isEmpty());
}

void WalletQmlModelTests::changeWalletPassphraseForwardsPasswords()
{
    FakeSecurityWallet* wallet{nullptr};
    auto model = MakeSecurityWalletModel(wallet);

    QVERIFY(model->changeWalletPassphrase("secret", "new-secret"));
    QCOMPARE(wallet->change_passphrase_calls, 1);
    QCOMPARE(wallet->changed_passphrases.size(), size_t{1});
    QCOMPARE(wallet->changed_passphrases.front().first, std::string("secret"));
    QCOMPARE(wallet->changed_passphrases.front().second, std::string("new-secret"));

    QVERIFY(!model->changeWalletPassphrase("wrong", "new-secret"));
    QCOMPARE(model->settingsError(), QString("The current wallet password was incorrect."));
}

void WalletQmlModelTests::backupWalletForwardsPath()
{
    FakeSecurityWallet* wallet{nullptr};
    auto model = MakeSecurityWalletModel(wallet);

    QVERIFY(model->backupWallet("/tmp/fake-wallet.bak"));
    QCOMPARE(wallet->backup_calls, 1);
    QCOMPARE(wallet->last_backup_path, std::string("/tmp/fake-wallet.bak"));
    QVERIFY(model->settingsError().isEmpty());
}

void WalletQmlModelTests::prepareTransactionOnLockedWalletMarksUnlockNeeded()
{
    ChainSelectionGuard chain_guard{ChainType::REGTEST};
    FakeSecurityWallet* wallet{nullptr};
    auto model = MakeSecurityWalletModel(wallet);
    ConfigureSecurityRecipient(*model, 1'000);

    wallet->create_transaction_fn = [](const std::vector<wallet::CRecipient>&,
                                       const wallet::CCoinControl&,
                                       bool,
                                       std::optional<unsigned int>) -> util::Result<wallet::CreatedTransactionResult> {
        return util::Error{Untranslated("Transaction needs a change address, but we can't generate it. Error: Keypool ran out, please call keypoolrefill first")};
    };

    QVERIFY(!model->prepareTransaction());
    QVERIFY(model->isEncrypted());
    QVERIFY(model->isLocked());
    QVERIFY(model->transactionNeedsUnlock());
    QCOMPARE(model->transactionError(), QString("Transaction needs a change address, but we can't generate it. Error: Keypool ran out, please call keypoolrefill first"));
    QVERIFY(wallet->create_transaction_sign_args == std::vector<bool>{false});
    QCOMPARE(wallet->unlock_calls, 0);
    QCOMPARE(wallet->lock_calls, 0);
}

void WalletQmlModelTests::sendTransactionOnLockedWalletRequiresPassword()
{
    ChainSelectionGuard chain_guard{ChainType::REGTEST};
    FakeSecurityWallet* wallet{nullptr};
    auto model = MakeSecurityWalletModel(wallet);
    ConfigureSecurityRecipient(*model, 1'000);

    QVERIFY(model->prepareTransactionWithPassphrase("secret"));
    QVERIFY(wallet->locked);
    QCOMPARE(wallet->unlock_calls, 1);
    QCOMPARE(wallet->lock_calls, 1);

    QVERIFY(!model->sendTransaction());
    QCOMPARE(model->transactionError(), QString("Enter your wallet password to sign this transaction."));
    QCOMPARE(wallet->commit_calls, 0);
    QVERIFY(wallet->fill_psbt_sign_args.empty());
}

void WalletQmlModelTests::sendTransactionWithPassphraseUnlocksCommitsAndRelocks()
{
    ChainSelectionGuard chain_guard{ChainType::REGTEST};
    FakeSecurityWallet* wallet{nullptr};
    auto model = MakeSecurityWalletModel(wallet);
    ConfigureSecurityRecipient(*model, 1'000);

    QVERIFY(model->prepareTransactionWithPassphrase("secret"));
    QVERIFY(wallet->locked);

    QVERIFY(model->sendTransactionWithPassphrase("secret"));
    QCOMPARE(wallet->unlock_calls, 2);
    QCOMPARE(wallet->lock_calls, 2);
    QCOMPARE(wallet->commit_calls, 1);
    QVERIFY(wallet->locked);
    QVERIFY(wallet->fill_psbt_sign_args == std::vector<bool>({false, true}));
    QVERIFY(model->transactionError().isEmpty());
    QVERIFY(!model->transactionNeedsUnlock());
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
