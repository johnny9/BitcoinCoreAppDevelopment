#ifndef BITCOIN_QML_MODELS_ACTIVITYLISTMODEL_H
#define BITCOIN_QML_MODELS_ACTIVITYLISTMODEL_H

#include <QAbstractListModel>
#include <QVector>
#include <QSharedPointer>
#include <QString>

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

    explicit Transaction(
        const QString &timestamp,
        const QString &address,
        const QString &label,
        const QString &amount,
        const QString &txid,
        Status status,
        Type type,
        QObject *parent = nullptr
    )
        : QObject(parent)
        , timestamp(timestamp)
        , address(address)
        , amount(amount)
        , label(label)
        , txid(txid)
        , status(status)
        , type(type) // default
    {
    }

    QString timestamp;
    QString address;
    QString amount;
    QString label;
    QString txid;
    Status status;
    Type type;
};

class ActivityListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ActivityListModel(QObject *parent = nullptr);

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
    QVector<QSharedPointer<Transaction>> m_transactions;
};

#endif // BITCOIN_QML_MODELS_ACTIVITYLISTMODEL_H
