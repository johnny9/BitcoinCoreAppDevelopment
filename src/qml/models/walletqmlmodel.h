// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QML_MODELS_WALLETQMLMODEL_H
#define BITCOIN_QML_MODELS_WALLETQMLMODEL_H

#include <interfaces/wallet.h>

#include <qml/models/paymentrequest.h>

#include <QObject>
#include <vector>

class WalletQmlModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString balance READ balance NOTIFY balanceChanged)

public:
    WalletQmlModel(std::unique_ptr<interfaces::Wallet> wallet, QObject *parent = nullptr);
    WalletQmlModel(QObject *parent = nullptr);
    ~WalletQmlModel();

    QString name() const;
    QString balance() const;
    Q_INVOKABLE PaymentRequest* createPaymentRequest(const QString& amount,
                                                     const QString& label,
                                                     const QString& message);

Q_SIGNALS:
    void nameChanged();
    void balanceChanged();

private:
    std::unique_ptr<interfaces::Wallet> m_wallet;
    std::vector<PaymentRequest*> m_payment_requests;
};

#endif // BITCOIN_QML_MODELS_WALLETQMLMODEL_H
