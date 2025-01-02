// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QML_MODELS_ACTIVITYLISTMODEL_H
#define BITCOIN_QML_MODELS_ACTIVITYLISTMODEL_H

#include <interfaces/wallet.h>
#include <qml/models/walletqmlmodel.h>
#include <qml/models/transaction.h>

#include <QAbstractListModel>
#include <QList>
#include <QSharedPointer>
#include <QString>

class WalletQmlModel;

class ActivityListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ActivityListModel(WalletQmlModel * parent = nullptr);

    enum TransactionRoles {
        AmountRole = Qt::UserRole + 1,
        AddressRole,
        LabelRole,
        DateTimeRole,
        StatusRole,
        TypeRole
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    void refreshWallet();
    void updateTransactionStatus(QSharedPointer<Transaction> tx) const;

private:
    QList<QSharedPointer<Transaction>> m_transactions;
    WalletQmlModel* m_wallet_model;
};

#endif // BITCOIN_QML_MODELS_ACTIVITYLISTMODEL_H
