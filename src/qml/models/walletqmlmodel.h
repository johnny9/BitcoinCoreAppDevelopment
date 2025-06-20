// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QML_MODELS_WALLETQMLMODEL_H
#define BITCOIN_QML_MODELS_WALLETQMLMODEL_H

#include <qml/models/activitylistmodel.h>
#include <qml/models/coinslistmodel.h>
#include <qml/models/paymentrequest.h>
#include <qml/models/sendrecipient.h>
#include <qml/models/sendrecipientslistmodel.h>

#include <QtCore/QTimer>
#include <consensus/amount.h>
#include <interfaces/handler.h>
#include <interfaces/wallet.h>
#include <uint256.h>
#include <wallet/coincontrol.h>
#include <wallet/wallet.h>

#include <memory>
#include <vector>

#include <QObject>

class WalletQmlModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString balance READ balance NOTIFY balanceChanged)
    Q_PROPERTY(ActivityListModel* activityListModel READ activityListModel CONSTANT)
    Q_PROPERTY(CoinsListModel* coinsListModel READ coinsListModel CONSTANT)
    Q_PROPERTY(SendRecipientsListModel* recipients READ sendRecipientList CONSTANT)
    Q_PROPERTY(unsigned int targetBlocks READ feeTargetBlocks WRITE setFeeTargetBlocks NOTIFY feeTargetBlocksChanged)
    Q_PROPERTY(PaymentRequest* currentPaymentRequest READ currentPaymentRequest CONSTANT)
    Q_PROPERTY(bool isWalletLoaded READ isWalletLoaded NOTIFY walletIsLoadedChanged)

public:
    WalletQmlModel(std::unique_ptr<interfaces::Wallet> wallet, QObject* parent = nullptr);
    WalletQmlModel(QObject *parent = nullptr);
    ~WalletQmlModel();

    QString name() const;
    QString balance() const;
    CAmount balanceSatoshi() const;
    Q_INVOKABLE void commitPaymentRequest();
    PaymentRequest* currentPaymentRequest() const { return m_current_payment_request; }

    ActivityListModel* activityListModel() const { return m_activity_list_model; }
    CoinsListModel* coinsListModel() const { return m_coins_list_model; }
    SendRecipientsListModel* sendRecipientList() const { return m_send_recipients; }
    Q_INVOKABLE bool prepareTransaction();
    Q_INVOKABLE void sendTransaction();

    // Returns wallet label for an address if present in address book, otherwise empty.
    Q_INVOKABLE QString addressLabel(const QString& address) const;

    std::set<interfaces::WalletTx> getWalletTxs() const;
    interfaces::WalletTx getWalletTx(const uint256& hash) const;
    bool tryGetTxStatus(const uint256& txid,
                        interfaces::WalletTxStatus& tx_status,
                        int& num_blocks,
                        int64_t& block_time) const;

    using TransactionChangedFn = std::function<void(const uint256& txid, ChangeType status)>;
    virtual std::unique_ptr<interfaces::Handler> handleTransactionChanged(TransactionChangedFn fn);

    interfaces::Wallet::CoinsList listCoins() const;
    bool lockCoin(const COutPoint& output);
    bool unlockCoin(const COutPoint& output);
    bool isLockedCoin(const COutPoint& output);
    void listLockedCoins(std::vector<COutPoint>& outputs);
    void selectCoin(const COutPoint& output);
    void unselectCoin(const COutPoint& output);
    bool isSelectedCoin(const COutPoint& output);
    std::vector<COutPoint> listSelectedCoins() const;
    unsigned int feeTargetBlocks() const;
    void setFeeTargetBlocks(unsigned int target_blocks);

    bool isWalletLoaded() const { return m_is_wallet_loaded; }
    void setWalletLoaded(bool loaded);

Q_SIGNALS:
    void nameChanged();
    void balanceChanged();
    void feeTargetBlocksChanged();
    void walletIsLoadedChanged();

public:
    // Periodically called to update balance and transaction confirmations
    void pollBalanceChanged();

private:
    static unsigned int m_next_payment_request_id;

    std::unique_ptr<interfaces::Wallet> m_wallet;
    PaymentRequest* m_current_payment_request{nullptr};
    ActivityListModel* m_activity_list_model{nullptr};
    CoinsListModel* m_coins_list_model{nullptr};
    SendRecipientsListModel* m_send_recipients{nullptr};
    CTransactionRef m_current_transaction{nullptr};
    wallet::CCoinControl m_coin_control;
    bool m_is_wallet_loaded{false};

    // Balance polling members
    interfaces::WalletBalances m_cached_balances;
    uint256 m_cached_last_update_tip;
    bool m_force_check_balance_changed{true};
    QTimer* m_poll_timer{nullptr};
};

#endif // BITCOIN_QML_MODELS_WALLETQMLMODEL_H
