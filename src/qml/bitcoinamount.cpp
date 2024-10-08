// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qml/bitcoinamount.h>

#include <QRegExp>
#include <QStringList>


BitcoinAmount::BitcoinAmount(QObject *parent) : QObject(parent)
{
}

QString BitcoinAmount::sanitize(const QString &text)
{
    QString result = text;

    // Remove any invalid characters
    result.remove(QRegExp("[^0-9.]"));

    // Ensure only one decimal point
    QStringList parts = result.split('.');
    if (parts.size() > 2) {
        result = parts[0] + "." + parts[1];
    }

    // Limit decimal places to 8
    if (parts.size() == 2 && parts[1].length() > 8) {
        result = parts[0] + "." + parts[1].left(8);
    }

    return result;
}