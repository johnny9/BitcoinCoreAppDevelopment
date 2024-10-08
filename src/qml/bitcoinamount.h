// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QML_BITCOINAMOUNT_H
#define BITCOIN_QML_BITCOINAMOUNT_H

#include <QObject>
#include <QString>

class BitcoinAmount : public QObject
{
    Q_OBJECT

public:
    explicit BitcoinAmount(QObject *parent = nullptr);

public Q_SLOTS:
    QString sanitize(const QString &text);
};

#endif // BITCOIN_QML_BITCOINAMOUNTS_H
