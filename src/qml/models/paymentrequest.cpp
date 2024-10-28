// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qml/models/paymentrequest.h>

PaymentRequest::PaymentRequest(QObject *parent)
    : QObject(parent)
{
}

QString PaymentRequest::address() const
{
    return m_address;
}

void PaymentRequest::setAddress(const QString &address)
{
    if (m_address == address)
        return;

    m_address = address;
    Q_EMIT addressChanged();
}

QString PaymentRequest::label() const
{
    return m_label;
}

void PaymentRequest::setLabel(const QString &label)
{
    if (m_label == label)
        return;

    m_label = label;
    Q_EMIT labelChanged();
}

QString PaymentRequest::message() const
{
    return m_message;
}

void PaymentRequest::setMessage(const QString &message)
{
    if (m_message == message)
        return;

    m_message = message;
    Q_EMIT messageChanged();
}

QString PaymentRequest::amount() const
{
    return m_amount;
}

void PaymentRequest::setAmount(const QString &amount)
{
    if (m_amount == amount)
        return;

    m_amount = amount;
    Q_EMIT amountChanged();
}
