// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qml/models/transaction.h>

#include <interfaces/wallet.h>
#include <key_io.h>
#include <wallet/types.h>

using wallet::ISMINE_SPENDABLE;
using wallet::isminetype;

Transaction::Transaction(
    const QString &timestamp,
    const QString &address,
    const QString &label,
    const QString &amount,
    const QString &txid,
    Status status,
    Type type,
    QObject *parent
)
    : QObject(parent)
    , address(address)
    , amount(amount)
    , label(label)
    , status(status)
    , timestamp(timestamp)
    , type(type)
    , txid(txid)
{
}

Transaction::Transaction(
    uint256 hash,
    qint64 time,
    Type type,
    const QString& address,
    CAmount debit,
    CAmount credit)
    : address(address)
    , credit(credit)
    , debit(debit)
    , hash(hash)
    , time(time)
    , type(type)
{
}

Transaction::Transaction(uint256 hash, qint64 time)
    : hash(hash)
    , time(time)
{
}

QList<QSharedPointer<Transaction>> Transaction::fromWalletTx(const interfaces::WalletTx& wtx)
{
    QList<QSharedPointer<Transaction>> parts;
    int64_t nTime = wtx.time;
    CAmount nCredit = wtx.credit;
    CAmount nDebit = wtx.debit;
    CAmount nNet = nCredit - nDebit;
    uint256 hash = wtx.tx->GetHash();
    std::map<std::string, std::string> mapValue = wtx.value_map;

    if (nNet > 0 || wtx.is_coinbase)
    {
        //
        // Credit
        //
        for(unsigned int i = 0; i < wtx.tx->vout.size(); i++)
        {
            const CTxOut& txout = wtx.tx->vout[i];
            isminetype mine = wtx.txout_is_mine[i];
            if(mine)
            {
                QSharedPointer<Transaction> sub = QSharedPointer<Transaction>::create(hash, nTime);
                sub->idx = i; // vout index
                sub->credit = txout.nValue;
                if (wtx.txout_address_is_mine[i])
                {
                    // Received by Bitcoin Address
                    sub->type = Transaction::RecvWithAddress;
                    sub->address = QString::fromStdString(EncodeDestination(wtx.txout_address[i]));
                }
                else
                {
                    // Received by IP connection (deprecated features), or a multisignature or other non-simple transaction
                    sub->type = Transaction::RecvFromOther;
                    sub->address = QString::fromStdString(mapValue["from"]);
                }
                if (wtx.is_coinbase)
                {
                    // Generated
                    sub->type = Transaction::Generated;
                }

                parts.append(sub);
            }
        }
    }
    else
    {
        isminetype fAllFromMe = ISMINE_SPENDABLE;
        for (const isminetype mine : wtx.txin_is_mine)
        {
            if(fAllFromMe > mine) fAllFromMe = mine;
        }

        isminetype fAllToMe = ISMINE_SPENDABLE;
        for (const isminetype mine : wtx.txout_is_mine)
        {
            if(fAllToMe > mine) fAllToMe = mine;
        }

        if (fAllFromMe && fAllToMe)
        {
            // Payment to self
            std::string address;
            for (auto it = wtx.txout_address.begin(); it != wtx.txout_address.end(); ++it) {
                if (it != wtx.txout_address.begin()) address += ", ";
                address += EncodeDestination(*it);
            }

            CAmount nChange = wtx.change;
            parts.append(QSharedPointer<Transaction>::create(hash, nTime, Transaction::SendToSelf, QString::fromStdString(address), -(nDebit - nChange), nCredit - nChange));
        }
        else if (fAllFromMe)
        {
            //
            // Debit
            //
            CAmount nTxFee = nDebit - wtx.tx->GetValueOut();

            for (unsigned int nOut = 0; nOut < wtx.tx->vout.size(); nOut++)
            {
                const CTxOut& txout = wtx.tx->vout[nOut];
                QSharedPointer<Transaction> sub = QSharedPointer<Transaction>::create(hash, nTime);
                sub->idx = nOut;

                if(wtx.txout_is_mine[nOut])
                {
                    // Ignore parts sent to self, as this is usually the change
                    // from a transaction sent back to our own address.
                    continue;
                }

                if (!std::get_if<CNoDestination>(&wtx.txout_address[nOut]))
                {
                    // Sent to Bitcoin Address
                    sub->type = Transaction::SendToAddress;
                    sub->address = QString::fromStdString(EncodeDestination(wtx.txout_address[nOut]));
                }
                else
                {
                    // Sent to IP, or other non-address transaction like OP_EVAL
                    sub->type = Transaction::SendToOther;
                    sub->address = QString::fromStdString(mapValue["to"]);
                }

                CAmount nValue = txout.nValue;
                /* Add fee to first output */
                if (nTxFee > 0)
                {
                    nValue += nTxFee;
                    nTxFee = 0;
                }
                sub->debit = -nValue;

                parts.append(sub);
            }
        }
        else
        {
            //
            // Mixed debit transaction, can't break down payees
            //
            parts.append(QSharedPointer<Transaction>::create(hash, nTime, Transaction::Other, QString(""), nNet, CAmount(0)));
        }
    }

    return parts;
}