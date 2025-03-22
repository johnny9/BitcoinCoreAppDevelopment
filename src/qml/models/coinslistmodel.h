// Copyright (c) 2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QML_MODELS_COINSLISTMODEL_H
#define BITCOIN_QML_MODELS_COINSLISTMODEL_H

#include <interfaces/handler.h>
#include <interfaces/wallet.h>
#include <qml/models/transaction.h>

#include <QAbstractListModel>
#include <QList>
#include <QSharedPointer>
#include <QString>
#include <qobjectdefs.h>

class WalletQmlModel;

class CoinsListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int lockedCoinsCount READ lockedCoinsCount NOTIFY lockedCoinsCountChanged)

public:
    explicit CoinsListModel(WalletQmlModel * parent = nullptr);
    ~CoinsListModel();

    enum CoinsRoles {
        AddressRole = Qt::UserRole + 1,
        AmountRole,
        DateTimeRole,
        LabelRole,
        LockedRole
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

public Q_SLOTS:
    void update();
    void setSortBy(const QString &roleName);
    bool toggleCoinLock(const int index);
    unsigned int lockedCoinsCount() const;

Q_SIGNALS:
    void sortByChanged(const QString &roleName);
    void lockedCoinsCountChanged();

private:
    WalletQmlModel* m_wallet_model;
    std::unique_ptr<interfaces::Handler> m_handler_transaction_changed;
    std::vector<std::tuple<CTxDestination, COutPoint, interfaces::WalletTxOut>> m_coins;
    QString m_sort_by;
};

#endif // BITCOIN_QML_MODELS_ACTIVITYLISTMODEL_H