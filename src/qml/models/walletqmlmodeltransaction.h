// Copyright (c) 2011-2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QML_MODELS_WALLETQMLMODELTRANSACTION_H
#define BITCOIN_QML_MODELS_WALLETQMLMODELTRANSACTION_H

#include <primitives/transaction.h>
#include <qml/models/sendrecipient.h>

#include <consensus/amount.h>

#include <QObject>


class WalletQmlModelTransaction : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString address READ address CONSTANT)
    Q_PROPERTY(QString amount READ amount NOTIFY amountChanged)
    Q_PROPERTY(QString label READ label CONSTANT)
    Q_PROPERTY(QString fee READ fee NOTIFY feeChanged)
public:
    explicit WalletQmlModelTransaction(const SendRecipient* recipient, QObject* parent = nullptr);

    QString address() const;
    QString amount() const;
    QString fee() const;
    QString label() const;

    QList<SendRecipient> getRecipients() const;

    CTransactionRef& getWtx();
    void setWtx(const CTransactionRef&);

    unsigned int getTransactionSize();

    void setTransactionFee(const CAmount& newFee);
    CAmount getTransactionFee() const;

    CAmount getTotalTransactionAmount() const;

    void reassignAmounts(int nChangePosRet); // needed for the subtract-fee-from-amount feature

Q_SIGNALS:
    void addressChanged();
    void labelChanged();
    void amountChanged();
    void feeChanged();

private:
    CTransactionRef m_wtx;
    CAmount m_fee{0};
    CAmount m_total_amount{0};
    QString m_address;
    QString m_label;
    QString m_amount;
};

#endif // BITCOIN_QML_MODELS_WALLETQMLMODELTRANSACTION_H
