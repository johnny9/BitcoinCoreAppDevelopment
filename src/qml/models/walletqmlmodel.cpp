// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include <qml/models/walletqmlmodel.h>

#include <qml/models/activitylistmodel.h>
#include <qml/models/paymentrequest.h>

#include <outputtype.h>
#include <qt/bitcoinunits.h>

#include <key_io.h>

#include <QTimer>
#include <string>

WalletQmlModel::WalletQmlModel(std::unique_ptr<interfaces::Wallet> wallet, QObject *parent)
    : QObject(parent)
{
    m_wallet = std::move(wallet);
    m_activity_list_model = new ActivityListModel(this);
}

WalletQmlModel::WalletQmlModel(QObject *parent)
    : QObject(parent)
{
    m_activity_list_model = new ActivityListModel(this);
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