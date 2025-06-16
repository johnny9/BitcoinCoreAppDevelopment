
// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include <qml/models/walletqmlmodel.h>

#include <qml/bitcoinamount.h>
#include <qml/models/activitylistmodel.h>
#include <qml/models/paymentrequest.h>
#include <qml/models/sendrecipient.h>
#include <qml/models/sendrecipientslistmodel.h>

#include <consensus/amount.h>
#include <interfaces/wallet.h>
#include <key_io.h>
#include <outputtype.h>
#include <qt/bitcoinunits.h>
#include <wallet/coincontrol.h>
#include <wallet/wallet.h>
#include <wallet/types.h>
#include <uint256.h>

#include <QTimer>
#include <QList>
#include <string>
#include <memory>

unsigned int WalletQmlModel::m_next_payment_request_id{1};

WalletQmlModel::WalletQmlModel(std::unique_ptr<interfaces::Wallet> wallet, QObject *parent)
    : QObject(parent)
{
    m_wallet = std::move(wallet);
    m_activity_list_model = new ActivityListModel(this);
    m_coins_list_model = new CoinsListModel(this);
    m_send_recipients = new SendRecipientsListModel(this);
    m_current_payment_request = new PaymentRequest(this);

    // Initialize cached tip to current best block if possible
    if (m_wallet) {
        m_cached_last_update_tip = uint256(); // Removed getBestBlockHash
        m_wallet->tryGetBalances(m_cached_balances, m_cached_last_update_tip);
    }

    // Setup polling timer
    m_poll_timer = new QTimer(this);
    connect(m_poll_timer, &QTimer::timeout, this, &WalletQmlModel::pollBalanceChanged);
    m_poll_timer->start(1000);
}

WalletQmlModel::WalletQmlModel(QObject* parent)
    : QObject(parent)
{
    m_activity_list_model = new ActivityListModel(this);
    m_coins_list_model = new CoinsListModel(this);
    m_send_recipients = new SendRecipientsListModel(this);
    m_current_payment_request = new PaymentRequest(this);

    // Setup polling timer even if wallet is nullptr for now
    m_poll_timer = new QTimer(this);
    connect(m_poll_timer, &QTimer::timeout, this, &WalletQmlModel::pollBalanceChanged);
    m_poll_timer->start(1000); // 1 second similar to MODEL_UPDATE_DELAY
}

WalletQmlModel::~WalletQmlModel()
{
    if (m_poll_timer) {
        m_poll_timer->stop();
    }
    delete m_activity_list_model;
    delete m_coins_list_model;
    delete m_send_recipients;
    delete m_current_payment_request;
}

QString WalletQmlModel::balance() const
{
    if (!m_wallet) {
        return "0";
    }
    return BitcoinAmount::satsToRichBtcString(m_wallet->getBalance());
}

CAmount WalletQmlModel::balanceSatoshi() const
{
    if (!m_wallet) {
        return 0;
    }
    return m_wallet->getBalance();
}

QString WalletQmlModel::name() const
{
    if (!m_wallet) {
        return QString();
    }
    return QString::fromStdString(m_wallet->getWalletName());
}

void WalletQmlModel::commitPaymentRequest()
{
    if (!m_current_payment_request) {
        return;
    }

    if (m_current_payment_request->id().isEmpty()) {
        m_current_payment_request->setId(m_next_payment_request_id++);
    }

    if (m_current_payment_request->address().isEmpty()) {
        // TODO: handle issues with getting the new address (wallet unlock?)
        auto destination = m_wallet->getNewDestination(OutputType::BECH32,
            m_current_payment_request->label().toStdString()).value();
        std::string address = EncodeDestination(destination);
        m_current_payment_request->setDestination(destination);
    }

    m_wallet->setAddressReceiveRequest(
        m_current_payment_request->destination(),
        m_current_payment_request->id().toStdString(),
        m_current_payment_request->message().toStdString());
}

std::set<interfaces::WalletTx> WalletQmlModel::getWalletTxs() const
{
    if (!m_wallet) {
        return {};
    }
    return m_wallet->getWalletTxs();
}

interfaces::WalletTx WalletQmlModel::getWalletTx(const uint256& hash) const
{
    if (!m_wallet) {
        return {};
    }
    return m_wallet->getWalletTx(hash);
}

bool WalletQmlModel::tryGetTxStatus(const uint256& txid,
                                    interfaces::WalletTxStatus& tx_status,
                                    int& num_blocks,
                                    int64_t& block_time) const
{
    if (!m_wallet) {
        return false;
    }
    return m_wallet->tryGetTxStatus(txid, tx_status, num_blocks, block_time);
}

std::unique_ptr<interfaces::Handler> WalletQmlModel::handleTransactionChanged(TransactionChangedFn fn)
{
    if (!m_wallet) {
        return nullptr;
    }
    return m_wallet->handleTransactionChanged(fn);
}

bool WalletQmlModel::prepareTransaction()
{
    if (!m_wallet || !m_send_recipients || m_send_recipients->recipients().empty()) {
        return false;
    }

    std::vector<wallet::CRecipient> vecSend;
    CAmount total = 0;
    for (auto* recipient : m_send_recipients->recipients()) {
        CScript scriptPubKey = GetScriptForDestination(DecodeDestination(recipient->address()->address().toStdString()));
        wallet::CRecipient c_recipient = {scriptPubKey, recipient->cAmount(), recipient->subtractFeeFromAmount()};
        m_coin_control.m_feerate = CFeeRate(1000);
        vecSend.push_back(c_recipient);
        total += recipient->cAmount();
    }

    CAmount balance = m_wallet->getBalance();
    if (balance < total) {
        return false;
    }

    int nChangePosRet = -1;
    CAmount nFeeRequired = 0;
    const auto& res = m_wallet->createTransaction(vecSend, m_coin_control, true, nChangePosRet, nFeeRequired);
    if (res) {
        m_current_transaction = *res;
        m_send_recipients->setFee(nFeeRequired);
        return true;
    } else {
        return false;
    }
}

QString WalletQmlModel::addressLabel(const QString& address) const
{
    if (!m_wallet) return QString();
    CTxDestination dest = DecodeDestination(address.toStdString());
    std::string name;
    wallet::isminetype mine;
    wallet::AddressPurpose purpose;
    if (m_wallet->getAddress(dest, &name, &mine, &purpose)) {
        return QString::fromStdString(name);
    }
    return QString();
}

void WalletQmlModel::sendTransaction()
{
    if (!m_wallet || !m_current_transaction || !m_send_recipients) {
        return;
    }

    // Build order form (e.g. URI message parameters)
    interfaces::WalletOrderForm order_form;
    for (auto* rcp : m_send_recipients->recipients()) {
        if (!rcp->message().isEmpty()) {
            order_form.emplace_back("Message", rcp->message().toStdString());
        }
    }

    // Commit the transaction to the wallet
    interfaces::WalletValueMap value_map; // currently unused
    m_wallet->commitTransaction(m_current_transaction, value_map, std::move(order_form));

    // Synchronise address book labels with recipients
    for (auto* rcp : m_send_recipients->recipients()) {
        const QString addr_str = rcp->address()->address();
        if (addr_str.isEmpty()) {
            continue;
        }
        CTxDestination dest = DecodeDestination(addr_str.toStdString());
        std::string strLabel = rcp->label().toStdString();
        if (strLabel.empty()) {
            continue; // nothing to store
        }

        std::string name;
        wallet::isminetype is_mine;
        wallet::AddressPurpose purpose;
        if (!m_wallet->getAddress(dest, &name, &is_mine, &purpose)) {
            // New entry – store with purpose SEND
            m_wallet->setAddressBook(dest, strLabel, wallet::AddressPurpose::SEND);
        } else if (name != strLabel) {
            // Update existing label, keep existing purpose
            m_wallet->setAddressBook(dest, strLabel, {});
        }
    }

    // Trigger an immediate balance refresh so UI updates quickly
    m_force_check_balance_changed = true;
}

interfaces::Wallet::CoinsList WalletQmlModel::listCoins() const
{
    if (!m_wallet) {
        return {};
    }
    return m_wallet->listCoins();
}

bool WalletQmlModel::lockCoin(const COutPoint& output)
{
    if (!m_wallet) {
        return false;
    }
    return m_wallet->lockCoin(output, true);
}

bool WalletQmlModel::unlockCoin(const COutPoint& output)
{
    if (!m_wallet) {
        return false;
    }
    return m_wallet->unlockCoin(output);
}

bool WalletQmlModel::isLockedCoin(const COutPoint& output)
{
    if (!m_wallet) {
        return false;
    }
    return m_wallet->isLockedCoin(output);
}

void WalletQmlModel::listLockedCoins(std::vector<COutPoint>& outputs)
{
    if (!m_wallet) {
        return;
    }
    m_wallet->listLockedCoins(outputs);
}

void WalletQmlModel::selectCoin(const COutPoint& output)
{
    m_coin_control.Select(output);
}

void WalletQmlModel::unselectCoin(const COutPoint& output)
{
    m_coin_control.UnSelect(output);
}

bool WalletQmlModel::isSelectedCoin(const COutPoint& output)
{
    return m_coin_control.IsSelected(output);
}

std::vector<COutPoint> WalletQmlModel::listSelectedCoins() const
{
    return m_coin_control.ListSelected();
}

void WalletQmlModel::pollBalanceChanged()
{
    if (!m_wallet) {
        return;
    }

    // Try to get balances and block tip; return early if wallet is busy
    interfaces::WalletBalances new_balances;
    uint256 block_hash;
    if (!m_wallet->tryGetBalances(new_balances, block_hash)) {
        return;
    }

    // Return early if nothing should be updated
    if (!m_force_check_balance_changed && block_hash == m_cached_last_update_tip) {
        return;
    }

    // Either forced or new block detected – reset flag and update cached tip
    m_force_check_balance_changed = false;
    m_cached_last_update_tip = block_hash;

    // Check if any of the balance components changed
    if (new_balances.balanceChanged(m_cached_balances)) {
        m_cached_balances = new_balances;
        Q_EMIT balanceChanged();
    }

    // Refresh confirmation counts in the activity model so UI stays in sync
    if (m_activity_list_model) {
        m_activity_list_model->updateConfirmations();
    }
}

unsigned int WalletQmlModel::feeTargetBlocks() const
{
    return m_coin_control.m_confirm_target.value_or(wallet::DEFAULT_TX_CONFIRM_TARGET);
}

void WalletQmlModel::setFeeTargetBlocks(unsigned int target_blocks)
{
    if (m_coin_control.m_confirm_target != target_blocks) {
        m_coin_control.m_confirm_target = target_blocks;
        Q_EMIT feeTargetBlocksChanged();
    }
}
