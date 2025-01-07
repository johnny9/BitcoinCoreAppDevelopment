// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QML_MODELS_WALLETQMLMODEL_H
#define BITCOIN_QML_MODELS_WALLETQMLMODEL_H

#include "interfaces/handler.h"
#include <interfaces/wallet.h>

#include <qml/models/activitylistmodel.h>
#include <qml/models/paymentrequest.h>

#include <QObject>
#include <qobjectdefs.h>
#include <vector>

class ActivityListModel;

class WalletQmlModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString balance READ balance NOTIFY balanceChanged)
    Q_PROPERTY(ActivityListModel* activityListModel READ activityListModel CONSTANT)

public:
    WalletQmlModel(std::unique_ptr<interfaces::Wallet> wallet, QObject *parent = nullptr);
    WalletQmlModel(QObject *parent = nullptr);
    ~WalletQmlModel();

    QString name() const;
    QString balance() const;
    Q_INVOKABLE PaymentRequest* createPaymentRequest(const QString& amount,
                                                     const QString& label,
                                                     const QString& message);
    ActivityListModel* activityListModel() const { return m_activity_list_model; }

    std::set<interfaces::WalletTx> getWalletTxs() const;
    bool tryGetTxStatus(const uint256& txid,
                        interfaces::WalletTxStatus& tx_status,
                        int& num_blocks,
                        int64_t& block_time) const;

    using ShowProgressFn = std::function<void(const std::string& title, int progress)>;
    std::unique_ptr<interfaces::Handler> handleShowProgress(ShowProgressFn fn);
    using TransactionChangedFn = std::function<void(const uint256& txid, ChangeType status)>;
    virtual std::unique_ptr<interfaces::Handler> handleTransactionChanged(TransactionChangedFn fn);

Q_SIGNALS:
    void nameChanged();
    void balanceChanged();

private:
    std::unique_ptr<interfaces::Wallet> m_wallet;
    std::vector<PaymentRequest*> m_payment_requests;
    ActivityListModel* m_activity_list_model{nullptr};
};

#endif // BITCOIN_QML_MODELS_WALLETQMLMODEL_H
