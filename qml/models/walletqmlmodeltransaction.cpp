// Copyright (c) 2011-2026 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qml/models/walletqmlmodeltransaction.h>

#include <qml/bitcoinunits.h>
#include <qml/models/sendrecipient.h>
#include <qml/models/sendrecipientslistmodel.h>

#include <policy/policy.h>

WalletQmlModelTransaction::WalletQmlModelTransaction(const SendRecipientsListModel* recipient, QObject* parent)
    : QObject(parent),
      m_address(recipient->recipients().at(0)->address()->address()),
      m_amount(recipient->totalAmountSatoshi()),
      m_fee(0),
      m_fee_amount(new BitcoinAmount(this)),
      m_total_amount(new BitcoinAmount(this)),
      m_label(recipient->recipients().at(0)->label()),
      m_wtx(nullptr)
{
    const BitcoinAmount::Unit display_unit = recipient->count() == 1
        ? recipient->recipients().at(0)->amount()->unit()
        : BitcoinAmount::Unit::BTC;
    m_fee_amount->setUnit(display_unit);
    m_fee_amount->setSatoshi(m_fee);
    m_total_amount->setUnit(display_unit);
    m_total_amount->setSatoshi(m_amount);
}

QString WalletQmlModelTransaction::formatWithUnit(CAmount value, int display_unit)
{
    auto unit = (display_unit == 1) ? QmlBitcoinUnits::Unit::SAT : QmlBitcoinUnits::Unit::BTC;
    QString num = QmlBitcoinUnits::format(unit, value, false, QmlBitcoinUnits::SeparatorStyle::STANDARD);
    if (display_unit == 1) {
        return num + " " + ((qAbs(value) == 1) ? QStringLiteral("sat") : QStringLiteral("sats"));
    }
    return num + " ₿";
}

QString WalletQmlModelTransaction::amount() const
{
    return formatWithUnit(m_amount, m_display_unit);
}

QString WalletQmlModelTransaction::address() const
{
    return m_address;
}

QString WalletQmlModelTransaction::fee() const
{
    return formatWithUnit(m_fee, m_display_unit);
}

BitcoinAmount* WalletQmlModelTransaction::feeAmount() const
{
    return m_fee_amount;
}

QString WalletQmlModelTransaction::total() const
{
    return formatWithUnit(m_amount + m_fee, m_display_unit);
}

BitcoinAmount* WalletQmlModelTransaction::totalAmount() const
{
    return m_total_amount;
}

QString WalletQmlModelTransaction::label() const
{
    return m_label;
}

void WalletQmlModelTransaction::setDisplayUnit(int unit)
{
    if (unit != m_display_unit) {
        m_display_unit = unit;
        Q_EMIT amountChanged();
        Q_EMIT feeChanged();
        Q_EMIT totalChanged();
    }
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
    if (m_fee != newFee) {
        m_fee = newFee;
        m_fee_amount->setSatoshi(m_fee);
        m_total_amount->setSatoshi(m_amount + m_fee);
        Q_EMIT feeChanged();
        Q_EMIT totalChanged();
    }
}

CAmount WalletQmlModelTransaction::getTotalTransactionAmount() const
{
    return m_amount + m_fee;
}

void WalletQmlModelTransaction::reassignAmounts(int nChangePosRet)
{
    const CTransaction* wallet_transaction = m_wtx.get();
    if (!wallet_transaction) {
        return;
    }

    CAmount reassigned_amount = 0;
    for (size_t recipient_index = 0; recipient_index < wallet_transaction->vout.size(); ++recipient_index) {
        if (static_cast<int>(recipient_index) == nChangePosRet) {
            continue;
        }

        reassigned_amount += wallet_transaction->vout[recipient_index].nValue;
    }

    if (m_amount != reassigned_amount) {
        m_amount = reassigned_amount;
        Q_EMIT amountChanged();
        Q_EMIT totalChanged();
    }
}
