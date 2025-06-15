// Copyright (c) 2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QML_MODELS_BITCOINADDRESS_H
#define BITCOIN_QML_MODELS_BITCOINADDRESS_H

#include <QObject>
#include <QString>

class BitcoinAddress : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString address READ address NOTIFY addressChanged)
    Q_PROPERTY(QString formattedAddress READ formattedAddress NOTIFY formattedAddressChanged)

public:
    explicit BitcoinAddress(QObject *parent = nullptr);
    BitcoinAddress(const QString &address, QObject *parent = nullptr);

    static QString ellipsesAddress(const QString &address);
    static QString formattedAddress(const QString &address);

    QString address() const;
    bool isEmpty() const;
    QString formattedAddress() const;
    QString ellipsesAddress() const;
    Q_INVOKABLE int setAddress(const QString &address, int cursorPosition = 0);

    Q_SIGNALS:
    void addressChanged();
    void formattedAddressChanged();
    void ellipsesAddressChanged();

private:
    QString m_address;
    QString m_formattedAddress;
};

#endif // BITCOIN_QML_MODELS_BITCOINADDRESS_H
