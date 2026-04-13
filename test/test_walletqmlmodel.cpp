// Copyright (c) 2026 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QtTest/QtTest>

#include <interfaces/handler.h>
#include <interfaces/wallet.h>
#include <outputtype.h>
#include <qml/models/walletqmlmodel.h>
#include <util/translation.h>

#include <consensus/amount.h>
#include <uint256.h>

#include <map>
#include <memory>

namespace {
class ScopedHandler : public interfaces::Handler
{
public:
    explicit ScopedHandler(std::function<void()> cleanup = [] {}) : m_cleanup(std::move(cleanup)) {}
    ~ScopedHandler() override { disconnect(); }

    void disconnect() override
    {
        if (!m_connected) {
            return;
        }
        m_connected = false;
        m_cleanup();
    }

private:
    std::function<void()> m_cleanup;
    bool m_connected{true};
};

class FakeWallet : public interfaces::Wallet
{
public:
    CAmount m_balance{0};
    std::string m_name{"testwallet"};

    bool encryptWallet(const SecureString&) override { return false; }
    bool isCrypted() override { return false; }
    bool lock() override { return false; }
    bool unlock(const SecureString&) override { return false; }
    bool isLocked() override { return false; }
    bool changeWalletPassphrase(const SecureString&, const SecureString&) override { return false; }
    void abortRescan() override {}
    bool backupWallet(const std::string&) override { return false; }
    std::string getWalletName() override { return m_name; }
    util::Result<CTxDestination> getNewDestination(const OutputType, const std::string&) override
    {
        return util::Error{Untranslated("unused")};
    }
    bool getPubKey(const CScript&, const CKeyID&, CPubKey&) override { return false; }
    SigningResult signMessage(const std::string&, const PKHash&, std::string&) override
    {
        return SigningResult::PRIVATE_KEY_NOT_AVAILABLE;
    }
    bool isSpendable(const CTxDestination&) override { return false; }
    bool setAddressBook(const CTxDestination&, const std::string&, const std::optional<wallet::AddressPurpose>&) override { return false; }
    bool delAddressBook(const CTxDestination&) override { return false; }
    bool getAddress(const CTxDestination&, std::string*, wallet::isminetype*, wallet::AddressPurpose*) override { return false; }
    std::vector<interfaces::WalletAddress> getAddresses() override { return {}; }
    std::vector<std::string> getAddressReceiveRequests() override { return {}; }
    bool setAddressReceiveRequest(const CTxDestination&, const std::string&, const std::string&) override { return false; }
    util::Result<void> displayAddress(const CTxDestination&) override { return {}; }
    bool lockCoin(const COutPoint&, const bool) override { return false; }
    bool unlockCoin(const COutPoint&) override { return false; }
    bool isLockedCoin(const COutPoint&) override { return false; }
    void listLockedCoins(std::vector<COutPoint>& outputs) override { outputs.clear(); }
    util::Result<CTransactionRef> createTransaction(const std::vector<wallet::CRecipient>&, const wallet::CCoinControl&, bool, int&, CAmount&) override
    {
        return util::Error{Untranslated("unused")};
    }
    void commitTransaction(CTransactionRef, interfaces::WalletValueMap, interfaces::WalletOrderForm) override {}
    bool transactionCanBeAbandoned(const Txid&) override { return false; }
    bool abandonTransaction(const Txid&) override { return false; }
    bool transactionCanBeBumped(const Txid&) override { return false; }
    bool createBumpTransaction(const Txid&, const wallet::CCoinControl&, std::vector<bilingual_str>&, CAmount&, CAmount&, CMutableTransaction&) override { return false; }
    bool signBumpTransaction(CMutableTransaction&) override { return false; }
    bool commitBumpTransaction(const Txid&, CMutableTransaction&&, std::vector<bilingual_str>&, Txid&) override { return false; }
    CTransactionRef getTx(const Txid&) override { return {}; }
    interfaces::WalletTx getWalletTx(const Txid&) override { return {}; }
    std::set<interfaces::WalletTx> getWalletTxs() override { return {}; }
    bool tryGetTxStatus(const Txid&, interfaces::WalletTxStatus&, int&, int64_t&) override { return false; }
    interfaces::WalletTx getWalletTxDetails(const Txid&, interfaces::WalletTxStatus&, interfaces::WalletOrderForm&, bool&, int&) override { return {}; }
    std::optional<common::PSBTError> fillPSBT(std::optional<int>, bool, bool, size_t*, PartiallySignedTransaction&, bool&) override
    {
        return std::nullopt;
    }
    interfaces::WalletBalances getBalances() override { return {.balance = m_balance}; }
    bool tryGetBalances(interfaces::WalletBalances& balances, uint256&) override
    {
        balances.balance = m_balance;
        balances.unconfirmed_balance = 0;
        balances.immature_balance = 0;
        return true;
    }
    CAmount getBalance() override { return m_balance; }
    CAmount getAvailableBalance(const wallet::CCoinControl&) override { return m_balance; }
    wallet::isminetype txinIsMine(const CTxIn&) override { return static_cast<wallet::isminetype>(0); }
    wallet::isminetype txoutIsMine(const CTxOut&) override { return static_cast<wallet::isminetype>(0); }
    CAmount getDebit(const CTxIn&, wallet::isminefilter) override { return 0; }
    CAmount getCredit(const CTxOut&, wallet::isminefilter) override { return 0; }
    interfaces::Wallet::CoinsList listCoins() override { return {}; }
    std::vector<interfaces::WalletTxOut> getCoins(const std::vector<COutPoint>&) override { return {}; }
    CAmount getRequiredFee(unsigned int) override { return 0; }
    CAmount getMinimumFee(unsigned int, const wallet::CCoinControl&, int*, FeeReason*) override { return 0; }
    unsigned int getConfirmTarget() override { return 2; }
    bool hdEnabled() override { return true; }
    bool canGetAddresses() override { return true; }
    bool privateKeysDisabled() override { return false; }
    bool taprootEnabled() override { return true; }
    bool hasExternalSigner() override { return false; }
    OutputType getDefaultAddressType() override { return OutputType::BECH32; }
    CAmount getDefaultMaxTxFee() override { return 0; }
    void remove() override {}
    std::unique_ptr<interfaces::Handler> handleUnload(UnloadFn) override { return std::make_unique<ScopedHandler>(); }
    std::unique_ptr<interfaces::Handler> handleShowProgress(ShowProgressFn) override { return std::make_unique<ScopedHandler>(); }
    std::unique_ptr<interfaces::Handler> handleStatusChanged(StatusChangedFn fn) override
    {
        const int id = ++m_next_handler_id;
        m_status_changed_fns.emplace(id, std::move(fn));
        return std::make_unique<ScopedHandler>([this, id] { m_status_changed_fns.erase(id); });
    }
    std::unique_ptr<interfaces::Handler> handleAddressBookChanged(AddressBookChangedFn) override { return std::make_unique<ScopedHandler>(); }
    std::unique_ptr<interfaces::Handler> handleTransactionChanged(TransactionChangedFn fn) override
    {
        const int id = ++m_next_handler_id;
        m_transaction_changed_fns.emplace(id, std::move(fn));
        return std::make_unique<ScopedHandler>([this, id] { m_transaction_changed_fns.erase(id); });
    }
    std::unique_ptr<interfaces::Handler> handleCanGetAddressesChanged(CanGetAddressesChangedFn) override { return std::make_unique<ScopedHandler>(); }

    void EmitStatusChanged()
    {
        for (const auto& [_, fn] : m_status_changed_fns) {
            fn();
        }
    }

    void EmitTransactionChanged(ChangeType change = CT_UPDATED)
    {
        const Txid txid = Txid::FromUint256(uint256{1});
        for (const auto& [_, fn] : m_transaction_changed_fns) {
            fn(txid, change);
        }
    }

private:
    int m_next_handler_id{0};
    std::map<int, StatusChangedFn> m_status_changed_fns;
    std::map<int, TransactionChangedFn> m_transaction_changed_fns;
};
} // namespace

class WalletQmlModelTests : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void transactionChangedEmitsBalanceChanged();
};

void WalletQmlModelTests::transactionChangedEmitsBalanceChanged()
{
    auto wallet = std::make_unique<FakeWallet>();
    auto* wallet_ptr = wallet.get();
    wallet_ptr->m_balance = 50 * COIN;

    WalletQmlModel model(std::move(wallet));
    QSignalSpy balance_spy(&model, &WalletQmlModel::balanceChanged);

    QCOMPARE(model.balance(), QString("50.00000000"));

    wallet_ptr->m_balance = 75 * COIN;
    wallet_ptr->EmitTransactionChanged();

    QTRY_COMPARE(balance_spy.count(), 1);
    QCOMPARE(model.balance(), QString("75.00000000"));
}

#ifdef BITCOINQML_NO_TEST_MAIN
#include <test/qt_test_registry.h>
BITCOINQML_REGISTER_QT_TEST(WalletQmlModelTests)
#else
QTEST_MAIN(WalletQmlModelTests)
#endif
#include "test_walletqmlmodel.moc"
