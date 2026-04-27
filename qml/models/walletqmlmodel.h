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
#include <qml/models/walletqmlmodeltransaction.h>

#include <common/args.h>
#include <common/settings.h>
#include <consensus/amount.h>
#include <interfaces/handler.h>
#include <interfaces/wallet.h>
#include <univalue.h>
#include <wallet/coincontrol.h>

#include <memory>
#include <optional>
#include <vector>

#include <QHash>
#include <QObject>
#include <QThread>
#include <QTimer>

class WalletQmlModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString displayName READ displayName NOTIFY displayNameChanged)
    Q_PROPERTY(QString balance READ balance NOTIFY balanceChanged)
    Q_PROPERTY(qint64 balanceSatoshi READ balanceSatoshi NOTIFY balanceChanged)
    Q_PROPERTY(bool hasExternalSigner READ hasExternalSigner CONSTANT)
    Q_PROPERTY(ActivityListModel* activityListModel READ activityListModel CONSTANT)
    Q_PROPERTY(CoinsListModel* coinsListModel READ coinsListModel CONSTANT)
    Q_PROPERTY(SendRecipientsListModel* recipients READ sendRecipientList CONSTANT)
    Q_PROPERTY(PaymentRequest* currentPaymentRequest READ currentPaymentRequest CONSTANT)
    Q_PROPERTY(WalletQmlModelTransaction* currentTransaction READ currentTransaction NOTIFY currentTransactionChanged)
    Q_PROPERTY(unsigned int targetBlocks READ feeTargetBlocks WRITE setFeeTargetBlocks NOTIFY feeTargetBlocksChanged)
    Q_PROPERTY(QString estimatedFee READ estimatedFee NOTIFY estimatedFeeChanged)
    Q_PROPERTY(bool customFeeEnabled READ customFeeEnabled WRITE setCustomFeeEnabled NOTIFY customFeeEnabledChanged)
    Q_PROPERTY(QString customFeeRate READ customFeeRate WRITE setCustomFeeRate NOTIFY customFeeRateChanged)
    Q_PROPERTY(bool customFeeRateValid READ customFeeRateValid NOTIFY customFeeRateValidChanged)
    Q_PROPERTY(bool feeEstimatePending READ feeEstimatePending NOTIFY feeEstimatePendingChanged)
    Q_PROPERTY(int feeEstimateRevision READ feeEstimateRevision NOTIFY feeEstimateRevisionChanged)
    Q_PROPERTY(bool isWalletLoaded READ isWalletLoaded NOTIFY walletIsLoadedChanged)
    Q_PROPERTY(int displayUnit READ displayUnit WRITE setDisplayUnit NOTIFY displayUnitChanged)
    Q_PROPERTY(bool isEncrypted READ isEncrypted NOTIFY securityStateChanged)
    Q_PROPERTY(bool isLocked READ isLocked NOTIFY securityStateChanged)
    Q_PROPERTY(QString keyScheme READ keyScheme CONSTANT)
    Q_PROPERTY(QString privateKeysStatus READ privateKeysStatus CONSTANT)
    Q_PROPERTY(QString externalSignerStatus READ externalSignerStatus CONSTANT)
    Q_PROPERTY(QString transactionError READ transactionError NOTIFY transactionErrorChanged)
    Q_PROPERTY(bool transactionNeedsUnlock READ transactionNeedsUnlock NOTIFY transactionNeedsUnlockChanged)
    Q_PROPERTY(QString settingsError READ settingsError NOTIFY settingsErrorChanged)

public:
    WalletQmlModel(std::unique_ptr<interfaces::Wallet> wallet, QObject* parent = nullptr);
    WalletQmlModel(QObject *parent = nullptr);
    ~WalletQmlModel();

    QString name() const;
    QString displayName() const;
    void setDisplayName(const QString& display_name);
    QString balance() const;
    qint64 balanceSatoshi() const;
    bool hasExternalSigner() const { return m_wallet && m_wallet->hasExternalSigner(); }
    Q_INVOKABLE void commitPaymentRequest();

    ActivityListModel* activityListModel() const { return m_activity_list_model; }
    CoinsListModel* coinsListModel() const { return m_coins_list_model; }
    SendRecipientsListModel* sendRecipientList() const { return m_send_recipients; }
    PaymentRequest* currentPaymentRequest() const { return m_current_payment_request; }
    WalletQmlModelTransaction* currentTransaction() const { return m_current_transaction; }
    QString estimatedFee() const;
    bool customFeeEnabled() const { return m_custom_fee_enabled; }
    QString customFeeRate() const { return m_custom_fee_rate; }
    bool customFeeRateValid() const;
    bool feeEstimatePending() const { return m_fee_estimate_pending; }
    int feeEstimateRevision() const { return m_fee_estimate_revision; }
    Q_INVOKABLE bool prepareTransaction();
    Q_INVOKABLE bool prepareTransactionWithPassphrase(const QString& passphrase);
    Q_INVOKABLE void approveExternalSignerTransaction();
    Q_INVOKABLE bool sendTransaction();
    Q_INVOKABLE bool sendTransactionWithPassphrase(const QString& passphrase);
    Q_INVOKABLE QString newAddress(QString label);
    Q_INVOKABLE QString estimatedFeeForTarget(unsigned int target_blocks) const;
    Q_INVOKABLE int feeTargetIndex(unsigned int target_blocks) const;
    Q_INVOKABLE void scheduleFeeEstimates();
    Q_INVOKABLE bool encryptWallet(const QString& passphrase);
    Q_INVOKABLE bool changeWalletPassphrase(const QString& old_passphrase, const QString& new_passphrase);
    Q_INVOKABLE bool backupWallet(const QString& path);
    Q_INVOKABLE void clearSettingsError();
    void removeWallet();

    std::set<interfaces::WalletTx> getWalletTxs() const;
    interfaces::WalletTx getWalletTx(const uint256& hash) const;
    bool tryGetTxStatus(const uint256& txid,
                        interfaces::WalletTxStatus& tx_status,
                        int& num_blocks,
                        int64_t& block_time) const;
    QString getAddressLabel(const QString& address) const;

    using TransactionChangedFn = std::function<void(const uint256& txid, ChangeType status)>;
    virtual std::unique_ptr<interfaces::Handler> handleTransactionChanged(TransactionChangedFn fn);
    using StatusChangedFn = std::function<void()>;
    virtual std::unique_ptr<interfaces::Handler> handleStatusChanged(StatusChangedFn fn);

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
    void setCustomFeeEnabled(bool enabled);
    void setCustomFeeRate(const QString& fee_rate);

    bool isWalletLoaded() const { return m_is_wallet_loaded; }
    void setWalletLoaded(bool loaded);
    int displayUnit() const { return m_display_unit; }
    void setDisplayUnit(int unit);
    bool isEncrypted() const { return m_is_encrypted; }
    bool isLocked() const { return m_is_locked; }
    QString keyScheme() const;
    QString privateKeysStatus() const;
    QString externalSignerStatus() const;
    QString transactionError() const { return m_transaction_error; }
    bool transactionNeedsUnlock() const { return m_transaction_needs_unlock; }
    QString settingsError() const { return m_settings_error; }

Q_SIGNALS:
    void nameChanged();
    void displayNameChanged();
    void balanceChanged();
    void currentTransactionChanged();
    void feeTargetBlocksChanged();
    void estimatedFeeChanged();
    void customFeeEnabledChanged();
    void customFeeRateChanged();
    void customFeeRateValidChanged();
    void feeEstimatePendingChanged();
    void feeEstimateRevisionChanged();
    void walletIsLoadedChanged();
    void displayUnitChanged(int unit);
    void externalSignerApprovalSucceeded();
    void externalSignerApprovalFailed(const QString& message, bool signerNotFound);
    void securityStateChanged();
    void transactionErrorChanged();
    void transactionNeedsUnlockChanged();
    void settingsErrorChanged();

private:
    void initializeFeeEstimator();
    void requestFeeEstimatesNow();
    void applyFeeEstimates(const QHash<unsigned int, QString>& estimates,
                           const QString& custom_estimate,
                           quint64 request_id);
    void clearFeeEstimates();
    QString ensurePreviewChangeAddress();
    unsigned int nextPaymentRequestId() const;
    void subscribeToWalletSignals();
    void unsubscribeFromWalletSignals();
    void refreshSecurityState();
    bool prepareTransactionInternal(const std::optional<QString>& passphrase);
    bool sendTransactionInternal(const std::optional<QString>& passphrase);
    bool unlockForAction(const std::optional<QString>& passphrase, bool& relock);
    void clearTransactionStatus();
    void setTransactionStatus(const QString& error, bool needs_unlock = false);
    void setSettingsError(const QString& error);

    std::unique_ptr<interfaces::Wallet> m_wallet;
    ActivityListModel* m_activity_list_model{nullptr};
    CoinsListModel* m_coins_list_model{nullptr};
    SendRecipientsListModel* m_send_recipients{nullptr};
    PaymentRequest* m_current_payment_request{nullptr};
    WalletQmlModelTransaction* m_current_transaction{nullptr};
    wallet::CCoinControl m_coin_control;
    QObject* m_fee_estimation_worker{nullptr};
    QThread* m_fee_estimation_thread{nullptr};
    QTimer* m_fee_estimation_timer{nullptr};
    QHash<unsigned int, QString> m_fee_estimates;
    QString m_custom_fee_estimate;
    QString m_custom_fee_rate;
    QString m_preview_change_address;
    quint64 m_fee_estimate_request_id{0};
    int m_fee_estimate_revision{0};
    bool m_custom_fee_enabled{false};
    bool m_fee_estimate_pending{false};
    bool m_is_wallet_loaded{false};
    int m_display_unit{0};
    bool m_is_encrypted{false};
    bool m_is_locked{false};
    QString m_transaction_error;
    bool m_transaction_needs_unlock{false};
    QString m_settings_error;
    QString m_display_name;
    std::unique_ptr<interfaces::Handler> m_handler_status_changed;
    std::unique_ptr<interfaces::Handler> m_handler_transaction_changed;
};

#endif // BITCOIN_QML_MODELS_WALLETQMLMODEL_H
