// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QML_MODELS_TRANSACTION_H
#define BITCOIN_QML_MODELS_TRANSACTION_H

#include "uint256.h"
#include <interfaces/wallet.h>

#include <QObject>
#include <QString>
#include <QSharedPointer>
#include <qglobal.h>
#include <qobject.h>

class Transaction : public QObject
{
    Q_OBJECT

public:
    enum Type {
        Other,
        Generated,
        SendToAddress,
        SendToOther,
        RecvWithAddress,
        RecvFromOther,
        SendToSelf
    };
    Q_ENUM(Type)

    enum Status {
        Confirmed,
        Unconfirmed,
        Confirming,
        Conflicted,
        Abandoned,
        Immature,
        NotAccepted
    };
    Q_ENUM(Status)

    Transaction(
        const QString &timestamp,
        const QString &address,
        const QString &label,
        const QString &amount,
        const QString &txid,
        Status status,
        Type type,
        QObject *parent = nullptr
    );

    Transaction(uint256 hash,
                qint64 time,
                Type type,
                const QString& address,
                CAmount debit,
                CAmount credit);

    Transaction(uint256 hash, qint64 time);

    QString address;
    QString amount;
    CAmount credit;
    CAmount debit;
    uint256 hash;
    int idx;
    QString label;
    Status status;
    qint64 time;
    QString timestamp;
    Type type;
    QString txid;

    static QList<QSharedPointer<Transaction>> fromWalletTx(const interfaces::WalletTx& tx);
};

#endif // BITCOIN_QML_MODELS_TRANSACTION_H
