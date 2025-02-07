// Copyright (c) 2011-2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qml/models/walletqmlmodeltransaction.h>

#include <policy/policy.h>

WalletQmlModelTransaction::WalletQmlModelTransaction(const SendRecipient& recipient)
    : recipient(recipient)
{
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
    m_fee = newFee;
}

CAmount WalletQmlModelTransaction::getTotalTransactionAmount() const
{
    CAmount totalTransactionAmount = 0;
    for (const SendRecipient &rcp : recipients)
    {
        totalTransactionAmount += rcp.amount;
    }
    return totalTransactionAmount;
}
