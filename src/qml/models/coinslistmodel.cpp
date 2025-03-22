// Copyright (c) 2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "coins.h"
#include <qchar.h>
#include <qml/models/coinslistmodel.h>

#include <interfaces/wallet.h>
#include <key_io.h>
#include <qml/models/walletqmlmodel.h>
#include <qobjectdefs.h>
#include <vector>

CoinsListModel::CoinsListModel(WalletQmlModel * parent)
    : QAbstractListModel(parent)
    , m_wallet_model(parent)
{
}

CoinsListModel::~CoinsListModel() = default;

int CoinsListModel::rowCount(const QModelIndex &parent) const
{
    return m_coins.size();
}

QVariant CoinsListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(m_coins.size()))
        return QVariant();

    const auto& [destination, outpoint, coin] = m_coins.at(index.row());
    switch (role) {
        case AddressRole:
            return QString::fromStdString(EncodeDestination(destination));
        case AmountRole:
            return QString::fromStdString("0.00000000");
        case LabelRole:
            return QString::fromStdString("");
        case LockedRole:
            return m_wallet_model->isLockedCoin(outpoint);
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> CoinsListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[AmountRole] = "amount";
    roles[AddressRole] = "address";
    roles[DateTimeRole] = "date";
    roles[LabelRole] = "label";
    roles[LockedRole] = "locked";
    return roles;
}

void CoinsListModel::update()
{
    if (m_wallet_model == nullptr) {
        return;
    }
    auto coins_map = m_wallet_model->listCoins();
    beginResetModel();
    m_coins.clear();
    for (const auto& [destination, vec] : coins_map) {
        for (const auto& [outpoint, tx_out] : vec) {
            auto tuple = std::make_tuple(destination, outpoint, tx_out);
            m_coins.push_back(tuple);
        }
    }
    endResetModel();
}

void CoinsListModel::setSortBy(const QString &roleName)
{
    if (m_sort_by != roleName) {
        m_sort_by = roleName;
        // sort(RoleNameToIndex(roleName));
        Q_EMIT sortByChanged(roleName);
    }
}

bool CoinsListModel::toggleCoinLock(const int index)
{
    if (index < 0 || index >= static_cast<int>(m_coins.size())) {
        return false;
    }
    const auto& [destination, outpoint, coin] = m_coins.at(index);
    bool return_value;
    if (m_wallet_model->isLockedCoin(outpoint)) {
        return_value = m_wallet_model->unlockCoin(outpoint);
    } else {
        return_value = m_wallet_model->lockCoin(outpoint);
    }
    Q_EMIT lockedCoinsCountChanged();
    return return_value;
}

unsigned int CoinsListModel::lockedCoinsCount() const
{
    std::vector<COutPoint> lockedCoins;
    m_wallet_model->listLockedCoins(lockedCoins);
    return lockedCoins.size();
}

