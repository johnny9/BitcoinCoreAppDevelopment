// Copyright (c) 2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qml/models/bitcoinaddress.h>

BitcoinAddress::BitcoinAddress(QObject* parent)
    : QObject(parent)
{
    // Initialize with empty address
    setAddress("", 0);
}

BitcoinAddress::BitcoinAddress(const QString& address, QObject* parent)
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
    return ellipsesAddress(m_address);
}

int BitcoinAddress::setAddress(const QString& input, int cursorPosition)
{
    if (input == m_address) {
        return cursorPosition;
    }

    if (input.isEmpty()) {
        m_address = "";
        m_formattedAddress = "";
        Q_EMIT addressChanged();
        Q_EMIT formattedAddressChanged();
        Q_EMIT ellipsesAddressChanged();
        return 0;
    }

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
    QString fmt = BitcoinAddress::formattedAddress(raw);
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

QString BitcoinAddress::formattedAddress(const QString& address)
{
    QString fmt;
    fmt.reserve(address.length() + address.length() / 4);
    for (int i = 0; i < address.length(); ++i) {
        if (i > 0 && (i % 4) == 0) {
            fmt += QChar(' ');
        }
        fmt += address[i];
    }
    return fmt;
}

QString BitcoinAddress::ellipsesAddress(const QString& address)
{
    if (address.length() > 8) {
        QString left = address.left(4) + ' ' + address.mid(4, 4);

        QString right = address.mid(address.length() - 8, 4) + ' ' + address.right(4);

        return left + " ... " + right;
    } else {
        return address;
    }
}