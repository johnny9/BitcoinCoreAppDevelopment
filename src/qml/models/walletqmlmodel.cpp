// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <consensus/amount.h>
#include "wallet/coincontrol.h"
#include "wallet/wallet.h"
#include <qml/models/walletqmlmodel.h>

#include <qml/models/activitylistmodel.h>
#include <qml/models/paymentrequest.h>
#include <qml/models/sendrecipient.h>
#include <qml/models/walletqmlmodeltransaction.h>

#include <outputtype.h>
#include <qobjectdefs.h>
#include <qt/bitcoinunits.h>

#include <key_io.h>

#include <QTimer>
#include <QList>
#include <string>

WalletQmlModel::WalletQmlModel(std::unique_ptr<interfaces::Wallet> wallet, QObject *parent)
    : QObject(parent)
{
    m_wallet = std::move(wallet);
    m_activity_list_model = new ActivityListModel(this);
    m_current_recipient = new SendRecipient(this);
}

WalletQmlModel::WalletQmlModel(QObject *parent)
    : QObject(parent)
{
    m_activity_list_model = new ActivityListModel(this);
    m_current_recipient = new SendRecipient(this);
}

QString WalletQmlModel::balance() const
{
    if (!m_wallet) {
        return "0";
    }
    return BitcoinUnits::format(BitcoinUnits::Unit::BTC, m_wallet->getBalance());
}

QString WalletQmlModel::name() const
{
    if (!m_wallet) {
        return QString();
    }
    return QString::fromStdString(m_wallet->getWalletName());
}

PaymentRequest* WalletQmlModel::createPaymentRequest(const QString& amount,
                                                     const QString& label,
                                                     const QString& message)
{
    PaymentRequest* request = new PaymentRequest(this);
    request->setAmount(amount);
    request->setLabel(label);

    // TODO: handle issues with getting the new address (wallet unlock?)
    auto destination = m_wallet->getNewDestination(OutputType::BECH32M, label.toStdString()).value();
    // TODO: integrate with RecentRequestsTableModel
    std::string address = EncodeDestination(destination);
    m_wallet->setAddressReceiveRequest(destination, label.toStdString(), "");
    request->setAddress(QString::fromStdString(address));
    m_payment_requests.push_back(request);
    return request;
}

std::set<interfaces::WalletTx> WalletQmlModel::getWalletTxs() const
{
    if (!m_wallet) {
        return {};
    }
    return m_wallet->getWalletTxs();
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

WalletQmlModel::~WalletQmlModel()
{
    for (PaymentRequest* request : m_payment_requests) {
        delete request;
    }
    delete m_activity_list_model;
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
    if (!m_wallet || !m_current_recipient) {
        return false;
    }

    CScript scriptPubKey = GetScriptForDestination(DecodeDestination(m_current_recipient->address().toStdString()));
    wallet::CRecipient recipient = {scriptPubKey, m_current_recipient->cAmount(), m_current_recipient->subtractFeeFromAmount()};
    wallet::CCoinControl coinControl;
    coinControl.m_feerate = CFeeRate(1000);

    CAmount balance = m_wallet->getBalance();
    if (balance < recipient.nAmount) {
        return false;
    }

    std::vector<wallet::CRecipient> vecSend{recipient};
    int nChangePosRet = -1;
    CAmount nFeeRequired = 0;
    const auto& res = m_wallet->createTransaction(vecSend, coinControl, true, nChangePosRet, nFeeRequired);
    if (res) {
        if (m_current_transaction) {
            delete m_current_transaction;
        }
        CTransactionRef newTx = *res;
        m_current_transaction = new WalletQmlModelTransaction(m_current_recipient, this);
        m_current_transaction->setWtx(newTx);
        m_current_transaction->setTransactionFee(nFeeRequired);
        Q_EMIT currentTransactionChanged();
        return true;
    } else {
        return false;
    }
}

void WalletQmlModel::sendTransaction()
{
    if (!m_wallet || !m_current_transaction) {
        return;
    }

    CTransactionRef newTx = m_current_transaction->getWtx();
    if (!newTx) {
        return;
    }

    interfaces::WalletValueMap value_map;
    interfaces::WalletOrderForm order_form;
    m_wallet->commitTransaction(newTx, value_map, order_form);
}
