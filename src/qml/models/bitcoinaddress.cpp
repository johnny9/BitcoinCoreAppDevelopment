// Copyright (c) 2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qml/models/bitcoinaddress.h>

#include <qchar.h>

BitcoinAddress::BitcoinAddress(QObject *parent)
    : QObject(parent)
{
    // Initialize with empty address
    setAddress("", 0);
}

BitcoinAddress::BitcoinAddress(const QString &address, QObject *parent)
    : QObject(parent)
{
    setAddress(address, 0);
}

QString BitcoinAddress::address() const
{
    return m_address;
}

bool BitcoinAddress::isEmpty() const
{
    return m_address.isEmpty();
}

QString BitcoinAddress::formattedAddress() const
{
    return m_formattedAddress;
}

QString BitcoinAddress::ellipsesAddress() const
{
    if (m_address.length() > 8) {
        return m_address.left(8) + "..." + m_address.right(8);
    } else {
        return m_address;
    }
}

int BitcoinAddress::setAddress(const QString &input, int cursorPosition)
{
    // 1) Define the allowed Base58 characters
    static const QString base58Chars =
        QStringLiteral("0123456789"
                       "ABCDEFGHJKLMNPQRSTUVWXYZ"
                       "abcdefghijklmnopqrstuvwxyz");

    // 2) Count how many *valid* chars were before the old cursor
    int posInClean = 0;
    const int lenIn = qMin(cursorPosition, input.length());
    for (int i = 0; i < lenIn; ++i) {
        if (base58Chars.contains(input[i]))
            ++posInClean;
    }

    // 3) Build the raw (no-space) address, filtering and capping at 62 chars
    QString raw;
    raw.reserve(qMin(input.length(), 62));
    for (QChar c : input) {
        if (base58Chars.contains(c)) {
            raw += c;
            if (raw.length() >= 62)
                break;
        }
    }
    m_address = raw;

    // 4) Format into groups of 4 chars separated by spaces
    QString fmt;
    fmt.reserve(raw.length() + raw.length() / 4);
    for (int i = 0; i < raw.length(); ++i) {
        if (i > 0 && (i % 4) == 0)
            fmt += QChar(' ');
        fmt += raw[i];
    }
    m_formattedAddress = fmt;

    // 5) Fire your QML signals
    Q_EMIT addressChanged();
    Q_EMIT formattedAddressChanged();
    Q_EMIT ellipsesAddressChanged();

    // 6) Map back to a cursor position in the new formatted string
    int newCursor = 0, seen = 0;
    while (newCursor < fmt.length() && seen < posInClean) {
        if (fmt[newCursor] != QChar(' '))
            ++seen;
        ++newCursor;
    }
    return newCursor;
}
