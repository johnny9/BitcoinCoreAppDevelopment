// Copyright (c) 2011-2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qml/models/walletqmlmodeltransaction.h>

#include <policy/policy.h>
#include <qobject.h>

WalletQmlModelTransaction::WalletQmlModelTransaction(const SendRecipient* recipient, QObject* parent)
: QObject(parent)
, m_address(recipient->address())
, m_label(recipient->label())
, m_amount(recipient->amount())
{
    m_total_amount = recipient->cAmount();
}

QString WalletQmlModelTransaction::amount() const
{
    return m_amount;
}

QString WalletQmlModelTransaction::address() const
{
    return m_address;
}

QString WalletQmlModelTransaction::fee() const
{
    return QString::number(m_fee);
}

QString WalletQmlModelTransaction::label() const
{
    return m_label;
}

CTransactionRef& WalletQmlModelTransaction::getWtx()
{
    return m_wtx;
}

void WalletQmlModelTransaction::setWtx(const CTransactionRef& newTx)
{
    m_wtx = newTx;
}

CAmount WalletQmlModelTransaction::getTransactionFee() const
{
    return m_fee;
}

void WalletQmlModelTransaction::setTransactionFee(const CAmount& newFee)
{
    if (m_fee != newFee)
    {
        m_fee = newFee;
        Q_EMIT feeChanged();
    }
}

CAmount WalletQmlModelTransaction::getTotalTransactionAmount() const
{
    return m_total_amount;
}
