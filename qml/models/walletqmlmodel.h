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

#include <consensus/amount.h>
#include <interfaces/handler.h>
#include <interfaces/wallet.h>
#include <psbt.h>
#include <wallet/coincontrol.h>

#include <memory>
#include <vector>

#include <QObject>
#include <QStringList>

namespace interfaces {
class Node;
} // namespace interfaces

class WalletQmlModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString balance READ balance NOTIFY balanceChanged)
    Q_PROPERTY(ActivityListModel* activityListModel READ activityListModel CONSTANT)
    Q_PROPERTY(CoinsListModel* coinsListModel READ coinsListModel CONSTANT)
    Q_PROPERTY(SendRecipientsListModel* recipients READ sendRecipientList CONSTANT)
    Q_PROPERTY(PaymentRequest* currentPaymentRequest READ currentPaymentRequest CONSTANT)
    Q_PROPERTY(WalletQmlModelTransaction* currentTransaction READ currentTransaction NOTIFY currentTransactionChanged)
    Q_PROPERTY(unsigned int targetBlocks READ feeTargetBlocks WRITE setFeeTargetBlocks NOTIFY feeTargetBlocksChanged)
    Q_PROPERTY(bool isWalletLoaded READ isWalletLoaded NOTIFY walletIsLoadedChanged)
    Q_PROPERTY(bool importedPsbtLoaded READ importedPsbtLoaded NOTIFY importedPsbtChanged)
    Q_PROPERTY(QString importedPsbtMode READ importedPsbtMode NOTIFY importedPsbtChanged)
    Q_PROPERTY(QString importedPsbtStatus READ importedPsbtStatus NOTIFY importedPsbtChanged)
    Q_PROPERTY(QString importedPsbtError READ importedPsbtError NOTIFY importedPsbtChanged)
    Q_PROPERTY(QStringList importedPsbtSummary READ importedPsbtSummary NOTIFY importedPsbtChanged)
    Q_PROPERTY(bool importedPsbtCanSign READ importedPsbtCanSign NOTIFY importedPsbtChanged)
    Q_PROPERTY(bool importedPsbtCanBroadcast READ importedPsbtCanBroadcast NOTIFY importedPsbtChanged)
    Q_PROPERTY(bool importedPsbtComplete READ importedPsbtComplete NOTIFY importedPsbtChanged)
    Q_PROPERTY(int importedPsbtUnsignedInputCount READ importedPsbtUnsignedInputCount NOTIFY importedPsbtChanged)
    Q_PROPERTY(int importedPsbtCouldSignInputCount READ importedPsbtCouldSignInputCount NOTIFY importedPsbtChanged)

public:
    WalletQmlModel(std::unique_ptr<interfaces::Wallet> wallet, QObject* parent = nullptr);
    WalletQmlModel(std::unique_ptr<interfaces::Wallet> wallet, interfaces::Node* node, QObject* parent = nullptr);
    WalletQmlModel(QObject *parent = nullptr);
    ~WalletQmlModel();

    QString name() const;
    QString balance() const;
    CAmount balanceSatoshi() const;
    Q_INVOKABLE void commitPaymentRequest();

    ActivityListModel* activityListModel() const { return m_activity_list_model; }
    CoinsListModel* coinsListModel() const { return m_coins_list_model; }
    SendRecipientsListModel* sendRecipientList() const { return m_send_recipients; }
    PaymentRequest* currentPaymentRequest() const { return m_current_payment_request; }
    WalletQmlModelTransaction* currentTransaction() const { return m_current_transaction; }
    Q_INVOKABLE bool prepareTransaction();
    Q_INVOKABLE void sendTransaction();
    Q_INVOKABLE QString newAddress(QString label);
    Q_INVOKABLE QString importPsbtFromFile(const QString& path);
    Q_INVOKABLE void clearImportedPsbt();
    Q_INVOKABLE void signImportedPsbt();
    Q_INVOKABLE void broadcastImportedPsbt();
    Q_INVOKABLE void copyImportedPsbtToClipboard();
    Q_INVOKABLE void saveImportedPsbtToFile(const QString& path);

    std::set<interfaces::WalletTx> getWalletTxs() const;
    interfaces::WalletTx getWalletTx(const uint256& hash) const;
    bool tryGetTxStatus(const uint256& txid,
                        interfaces::WalletTxStatus& tx_status,
                        int& num_blocks,
                        int64_t& block_time) const;
    QString getAddressLabel(const QString& address) const;

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
    void setNode(interfaces::Node* node) { m_node = node; }

    bool importedPsbtLoaded() const { return m_imported_psbt || m_imported_psbt_mode == QStringLiteral("draft"); }
    QString importedPsbtMode() const { return m_imported_psbt_mode; }
    QString importedPsbtStatus() const { return m_imported_psbt_status; }
    QString importedPsbtError() const { return m_imported_psbt_error; }
    QStringList importedPsbtSummary() const { return m_imported_psbt_summary; }
    bool importedPsbtCanSign() const { return m_imported_psbt_can_sign; }
    bool importedPsbtCanBroadcast() const { return m_imported_psbt_can_broadcast; }
    bool importedPsbtComplete() const { return m_imported_psbt_complete; }
    int importedPsbtUnsignedInputCount() const { return m_imported_psbt_unsigned_inputs; }
    int importedPsbtCouldSignInputCount() const { return m_imported_psbt_could_sign_inputs; }

Q_SIGNALS:
    void nameChanged();
    void balanceChanged();
    void currentTransactionChanged();
    void feeTargetBlocksChanged();
    void walletIsLoadedChanged();
    void importedPsbtChanged();

private:
    unsigned int nextPaymentRequestId() const;
    QString importPsbt(PartiallySignedTransaction psbt);
    bool tryImportPsbtToReview(const PartiallySignedTransaction& psbt, QString& mode, QString& reason);
    void refreshImportedPsbtState(const QString& status_override = QString());
    void setImportedPsbtError(const QString& error);
    QStringList buildPsbtSummary(const PartiallySignedTransaction& psbt) const;

    std::unique_ptr<interfaces::Wallet> m_wallet;
    interfaces::Node* m_node{nullptr};
    ActivityListModel* m_activity_list_model{nullptr};
    CoinsListModel* m_coins_list_model{nullptr};
    SendRecipientsListModel* m_send_recipients{nullptr};
    PaymentRequest* m_current_payment_request{nullptr};
    WalletQmlModelTransaction* m_current_transaction{nullptr};
    wallet::CCoinControl m_coin_control;
    bool m_is_wallet_loaded{false};
    std::unique_ptr<PartiallySignedTransaction> m_imported_psbt;
    QString m_imported_psbt_mode;
    QString m_imported_psbt_status;
    QString m_imported_psbt_error;
    QStringList m_imported_psbt_summary;
    bool m_imported_psbt_can_sign{false};
    bool m_imported_psbt_can_broadcast{false};
    bool m_imported_psbt_complete{false};
    int m_imported_psbt_unsigned_inputs{0};
    int m_imported_psbt_could_sign_inputs{0};
};

#endif // BITCOIN_QML_MODELS_WALLETQMLMODEL_H
