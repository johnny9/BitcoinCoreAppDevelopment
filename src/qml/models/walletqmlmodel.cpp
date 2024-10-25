// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qml/models/walletqmlmodel.h>

#include <qt/bitcoinunits.h>

#include <key_io.h>

#include <QTimer>

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

PaymentRequest WalletQmlModel::createPaymentRequest(const QString& amount,
                                                    const QString& label,
                                                    const QString& message)
{
    PaymentRequest request;
    request.setAmount(amount);
    request.setLabel(label);

    // TODO: handle issues with getting the new address (wallet unlock?)
    auto destination = m_wallet->getNewDestination(OutputType.BECH32M, label.toStdString()).value();
    // TODO: integrate with RecentRequestsTableModel
    m_wallet->setAddressReceiveRequest(EncodeDestination(*destination), label.toStdString(), "");
    request.setAddress(QString::fromStdString(destination));
}
