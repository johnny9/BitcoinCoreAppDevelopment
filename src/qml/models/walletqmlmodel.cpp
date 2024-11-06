// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "qml/models/paymentrequest.h"
#include <qml/models/walletqmlmodel.h>

#include <outputtype.h>
#include <qt/bitcoinunits.h>

#include <key_io.h>

#include <QTimer>
#include <string>

WalletQmlModel::WalletQmlModel(std::unique_ptr<interfaces::Wallet> wallet, QObject *parent)
    : QObject(parent)
{
    m_wallet = std::move(wallet);
}

WalletQmlModel::WalletQmlModel(QObject *parent)
    : QObject(parent)
{
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

WalletQmlModel::~WalletQmlModel()
{
    for (PaymentRequest* request : m_payment_requests) {
        delete request;
    }
}
