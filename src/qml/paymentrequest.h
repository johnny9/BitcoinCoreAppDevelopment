// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QML_PAYMENTREQUEST_H
#define BITCOIN_QML_PAYMENTREQUEST_H

#include <QObject>

class PaymentRequest : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString address READ address WRITE setAddress NOTIFY addressChanged)
    Q_PROPERTY(QString label READ label WRITE setLabel NOTIFY labelChanged)
    Q_PROPERTY(QString message READ message WRITE setMessage NOTIFY messageChanged)
    Q_PROPERTY(QString amount READ amount WRITE setAmount NOTIFY amountChanged)

public:
    explicit PaymentRequest(QObject *parent = nullptr);

    QString address() const;
    void setAddress(const QString &address);

    QString label() const;
    void setLabel(const QString &label);

    QString message() const;
    void setMessage(const QString &message);

    QString amount() const;
    void setAmount(const QString &amount);

Q_SIGNALS:
    void addressChanged();
    void labelChanged();
    void messageChanged();
    void amountChanged();

private:
    QString m_address;
    QString m_label;
    QString m_message;
    QString m_amount;

}