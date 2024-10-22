// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QML_BITCOINAMOUNT_H
#define BITCOIN_QML_BITCOINAMOUNT_H

#include <QObject>
#include <QString>
#include <qobjectdefs.h>

class BitcoinAmount : public QObject
{
    enum class Unit {
        BTC,
        SAT
    };
    Q_ENUM(Unit)

    Q_OBJECT
    Q_PROPERTY(Unit unit READ unit WRITE setUnit NOTIFY unitChanged)
    Q_PROPERTY(QString unitLabel READ unitLabel WRITE setUnitLabel NOTIFY unitLabelChanged)

public:
    explicit BitcoinAmount(QObject *parent = nullptr);

    Unit unit() const;
    void setUnit(Unit unit);


public Q_SLOTS:
    QString sanitize(const QString &text);

Q_SIGNALS:
    void unitChanged();
    void unitLabelChanged();

private:
    Unit m_unit;
    QString m_unitLabel;
};

#endif // BITCOIN_QML_BITCOINAMOUNT_H
