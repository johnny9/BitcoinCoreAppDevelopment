// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qml/models/walletlistmodel.h>

#include <interfaces/node.h>

#include <QSet>
#include <QSettings>

namespace {
QString WalletDisplayName(const QString& path)
{
    const QString trimmed_path = path.trimmed();
    if (trimmed_path.isEmpty()) {
        return {};
    }

    QSettings settings;
    const QString display_name = settings.value(QStringLiteral("walletDisplayNames/%1").arg(trimmed_path)).toString().trimmed();
    return display_name.isEmpty() ? trimmed_path : display_name;
}
} // namespace

WalletListModel::WalletListModel(interfaces::Node& node, QObject *parent)
: QAbstractListModel(parent)
, m_node(node)
{
}

void WalletListModel::listWalletDir()
{
    beginResetModel();
    m_items.clear();
    for (const auto& [path, info] : m_node.walletLoader().listWalletDir()) {
        Q_UNUSED(info);
        QString qname = QString::fromStdString(path);
        m_items.append({ qname });
    }
    endResetModel();
}

void WalletListModel::setOpenWalletNames(const QStringList& wallet_names)
{
    const QSet<QString> updated_names{wallet_names.begin(), wallet_names.end()};
    if (m_open_wallet_names == updated_names) {
        return;
    }

    m_open_wallet_names = updated_names;
    updateLoadStateForAllRows();
}

int WalletListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_items.size();
}

QVariant WalletListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
        return QVariant();

    const auto &item = m_items[index.row()];
    switch (role) {
    case Qt::DisplayRole:
    case DisplayNameRole:
        return WalletDisplayName(item.name);
    case NameRole:
        return item.name;
    case LoadStateRole:
        return m_open_wallet_names.contains(item.name)
            ? static_cast<int>(LoadState::Open)
            : static_cast<int>(LoadState::Closed);
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> WalletListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[DisplayNameRole] = "displayName";
    roles[LoadStateRole] = "loadState";
    return roles;
}

void WalletListModel::addItem(const Item &item)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_items.append(item);
    endInsertRows();
}

void WalletListModel::updateLoadStateForAllRows()
{
    if (m_items.isEmpty()) {
        return;
    }

    const QModelIndex first = index(0, 0);
    const QModelIndex last = index(rowCount() - 1, 0);
    Q_EMIT dataChanged(first, last, {LoadStateRole});
}

void WalletListModel::refreshDisplayNames()
{
    if (m_items.isEmpty()) {
        return;
    }

    const QModelIndex first = index(0, 0);
    const QModelIndex last = index(rowCount() - 1, 0);
    Q_EMIT dataChanged(first, last, {Qt::DisplayRole, DisplayNameRole});
}
