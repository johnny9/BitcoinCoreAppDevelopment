#include <qml/models/activitylistmodel.h>

ActivityListModel::ActivityListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_transactions.append(QSharedPointer<Transaction>::create(
        "July 11",
        "bc1q wvlv mha3 cvhy q6qz tjzu mq2d 63ff htzy xxu6 q8",
        "Payment from Alex for freelance coding work.",
        "₿ +0.00 001 000",
        "abcd1234efgh5678...",
        Transaction::Unconfirmed,
        Transaction::RecvWithAddress
    ));

    m_transactions.append(QSharedPointer<Transaction>::create(
        "July 12",
        "bc1q wvlv mha3 cvhy q6qz tjzu mq2d 63ff htzy xxu6 q8",
        "Coffee from xpub.",
        "₿ -0.00 001 000",
        "mnop3456qrst7890...",
        Transaction::Confirmed,
        Transaction::SendToAddress
    ));

    m_transactions.append(QSharedPointer<Transaction>::create(
        "February 2",
        "bc1q wvlv mha3 cvhy q6qz tjzu mq2d 63ff htzy xxu6 q8",
        "Received rent payment from Lisa.",
        "₿ +0.00 001 000",
        "cdef6789abcd1234...",
        Transaction::Confirmed,
        Transaction::RecvWithAddress
    ));
}

int ActivityListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_transactions.size();
}

QVariant ActivityListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_transactions.size())
        return QVariant();

    QSharedPointer<Transaction> tx = m_transactions.at(index.row());

    switch (role) {
    case AmountRole:
        return tx->amount;
    case AddressRole:
        return tx->address;
    case LabelRole:
        return tx->label;
    case DateTimeRole:
        return tx->timestamp;
    case StatusRole:
        return tx->status;
    case TypeRole:
        return tx->type;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ActivityListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[AmountRole] = "amount";
    roles[AddressRole] = "address";
    roles[LabelRole] = "label";
    roles[DateTimeRole] = "date";
    roles[StatusRole] = "status";
    roles[TypeRole] = "type";
    return roles;
}
