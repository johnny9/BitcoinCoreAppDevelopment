// Copyright (c) 2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qml/models/sendrecipient.h>

#include <qml/bitcoinamount.h>
#include <qml/models/bitcoinaddress.h>
#include <qml/models/walletqmlmodel.h>

#include <key_io.h>

SendRecipient::SendRecipient(WalletQmlModel* wallet, QObject* parent)
    : QObject(parent), m_wallet(wallet), m_address(new BitcoinAddress(this)), m_amount(new BitcoinAmount(this))
{
    connect(m_amount, &BitcoinAmount::amountChanged, this, &SendRecipient::validateAmount);
    connect(m_address, &BitcoinAddress::formattedAddressChanged, this, &SendRecipient::validateAddress);
}

BitcoinAddress* SendRecipient::address() const
{
    return m_address;
}

void SendRecipient::setAddress(const QString& address)
{
    if (m_address->address() != address) {
        m_address->setAddress(address, 0);
        Q_EMIT addressChanged();
        validateAddress();
    }
}

QString SendRecipient::addressError() const
{
    return m_addressError;
}

void SendRecipient::setAddressError(const QString& error)
{
    if (m_addressError != error) {
        m_addressError = error;
        Q_EMIT addressErrorChanged();
    }
}

QString SendRecipient::label() const
{
    return m_label;
}

void SendRecipient::setLabel(const QString& label)
{
    if (m_label != label) {
        m_label = label;
        Q_EMIT labelChanged();
    }
}

BitcoinAmount* SendRecipient::amount() const
{
    return m_amount;
}

QString SendRecipient::amountError() const
{
    return m_amountError;
}

void SendRecipient::setAmountError(const QString& error)
{
    if (m_amountError != error) {
        m_amountError = error;
        Q_EMIT amountErrorChanged();
    }
}

QString SendRecipient::message() const
{
    return m_message;
}

void SendRecipient::setMessage(const QString& message)
{
    if (m_message != message) {
        m_message = message;
        Q_EMIT messageChanged();
    }
}

bool SendRecipient::subtractFeeFromAmount() const
{
    return m_subtractFeeFromAmount;
}

CAmount SendRecipient::cAmount() const
{
    return m_amount->satoshi();
}

void SendRecipient::clear()
{
    m_address->setAddress("", 0);
    m_label = "";
    m_amount->setSatoshi(0);
    m_message = "";
    m_subtractFeeFromAmount = false;
    Q_EMIT addressChanged();
    Q_EMIT labelChanged();
    Q_EMIT messageChanged();
    Q_EMIT amount() -> amountChanged();
}

void SendRecipient::validateAddress()
{
    if (!m_address->isEmpty() && !IsValidDestinationString(m_address->address().toStdString())) {
        if (IsValidDestinationString(m_address->address().toStdString(), *CChainParams::Main())) {
            setAddressError(tr("Address is valid for mainnet, not the current network"));
        } else if (IsValidDestinationString(m_address->address().toStdString(), *CChainParams::TestNet())) {
            setAddressError(tr("Address is valid for testnet, not the current network"));
        } else {
            setAddressError(tr("Invalid address format"));
        }
    } else {
        setAddressError("");
    }

    Q_EMIT isValidChanged();
}

void SendRecipient::validateAmount()
{
    if (m_amount->isSet()) {
        if (m_amount->satoshi() <= 0) {
            setAmountError(tr("Amount must be greater than zero"));
        } else if (m_amount->satoshi() > MAX_MONEY) {
            setAmountError(tr("Amount exceeds maximum limit of 21,000,000 BTC"));
        } else if (m_amount->satoshi() > m_wallet->balanceSatoshi()) {
            setAmountError(tr("Amount exceeds available balance"));
        } else {
            setAmountError("");
        }
    }

    Q_EMIT isValidChanged();
}

bool SendRecipient::isValid() const
{
    return m_addressError.isEmpty() && m_amountError.isEmpty() && m_amount->satoshi() > 0 && !m_address->isEmpty();
}
