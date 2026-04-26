
// Copyright (c) 2024-2026 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qml/models/walletqmlmodel.h>

#include <qml/models/activitylistmodel.h>
#include <qml/models/paymentrequest.h>
#include <qml/models/sendrecipient.h>
#include <qml/models/sendrecipientslistmodel.h>
#include <qml/models/walletqmlmodeltransaction.h>

#include <common/messages.h>
#include <common/types.h>
#include <consensus/amount.h>
#include <interfaces/node.h>
#include <interfaces/wallet.h>
#include <key_io.h>
#include <node/psbt.h>
#include <node/transaction.h>
#include <node/types.h>
#include <addresstype.h>
#include <outputtype.h>
#include <primitives/transaction.h>
#include <psbt.h>
#include <qml/bitcoinunits.h>
#include <script/solver.h>
#include <serialize.h>
#include <streams.h>
#include <util/strencodings.h>
#include <util/translation.h>
#include <wallet/coincontrol.h>
#include <wallet/wallet.h>

#include <QClipboard>
#include <QDateTime>
#include <QFile>
#include <QGuiApplication>
#include <QUrl>

#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace {
struct QmlReceiveRequestRecipient
{
    static constexpr int CURRENT_VERSION{1};
    int nVersion{CURRENT_VERSION};
    std::string address;
    std::string label;
    CAmount amount{0};
    std::string message;
    std::string sPaymentRequest;
    std::string authenticatedMerchant;

    SERIALIZE_METHODS(QmlReceiveRequestRecipient, obj)
    {
        READWRITE(obj.nVersion, obj.address, obj.label, obj.amount, obj.message, obj.sPaymentRequest, obj.authenticatedMerchant);
    }
};

struct QmlRecentRequestEntry
{
    static constexpr int CURRENT_VERSION{1};
    int nVersion{CURRENT_VERSION};
    int64_t id{0};
    QDateTime date;
    QmlReceiveRequestRecipient recipient;

    SERIALIZE_METHODS(QmlRecentRequestEntry, obj)
    {
        unsigned int date_timet;
        SER_WRITE(obj, date_timet = obj.date.toSecsSinceEpoch());
        READWRITE(obj.nVersion, obj.id, date_timet, obj.recipient);
        SER_READ(obj, obj.date = QDateTime::fromSecsSinceEpoch(date_timet));
    }
};


QString LocalFilePath(const QString& path)
{
    const QUrl url(path);
    if (url.isLocalFile()) {
        return url.toLocalFile();
    }
    return path;
}

QString FormatBtc(CAmount amount)
{
    return QmlBitcoinUnits::format(QmlBitcoinUnits::Unit::BTC, amount) + QStringLiteral(" BTC");
}

QString SerializePsbtBase64(const PartiallySignedTransaction& psbt)
{
    DataStream stream{};
    stream << psbt;
    return QString::fromStdString(EncodeBase64(stream.str()));
}

QByteArray SerializePsbtRaw(const PartiallySignedTransaction& psbt)
{
    DataStream stream{};
    stream << psbt;
    const std::string raw{stream.str()};
    return QByteArray(raw.data(), static_cast<qsizetype>(raw.size()));
}

bool IsMultisigScript(const CScript& script)
{
    if (script.empty()) {
        return false;
    }

    std::vector<std::vector<unsigned char>> solutions;
    return Solver(script, solutions) == TxoutType::MULTISIG || MatchMultiA(script).has_value();
}

bool IsMultisigPsbtInput(const PartiallySignedTransaction& psbt, size_t index)
{
    const PSBTInput& input{psbt.inputs[index]};
    if (IsMultisigScript(input.redeem_script) || IsMultisigScript(input.witness_script)) {
        return true;
    }

    CTxOut utxo;
    return psbt.GetInputUTXO(utxo, index) && IsMultisigScript(utxo.scriptPubKey);
}

bool DecodePsbtFromBytes(const QByteArray& bytes, PartiallySignedTransaction& psbt, std::string& error)
{
    std::vector<std::byte> raw;
    raw.reserve(bytes.size());
    for (const char ch : bytes) {
        raw.push_back(static_cast<std::byte>(static_cast<unsigned char>(ch)));
    }
    return DecodeRawPSBT(psbt, std::span<const std::byte>{raw.data(), raw.size()}, error);
}

QString PsbtErrorText(common::PSBTError error)
{
    return QString::fromStdString(common::PSBTErrorString(error).translated);
}

QString TransactionErrorText(node::TransactionError error)
{
    return QString::fromStdString(common::TransactionErrorString(error).translated);
}
} // namespace

WalletQmlModel::WalletQmlModel(std::unique_ptr<interfaces::Wallet> wallet, QObject *parent)
    : WalletQmlModel(std::move(wallet), nullptr, parent)
{
}

WalletQmlModel::WalletQmlModel(std::unique_ptr<interfaces::Wallet> wallet, interfaces::Node* node, QObject *parent)
    : QObject(parent)
    , m_wallet(std::move(wallet))
    , m_node(node)
{
    m_activity_list_model = new ActivityListModel(this);
    m_coins_list_model = new CoinsListModel(this);
    m_send_recipients = new SendRecipientsListModel(this);
    m_current_payment_request = new PaymentRequest(this);
}

WalletQmlModel::WalletQmlModel(QObject* parent)
    : QObject(parent)
{
    m_activity_list_model = new ActivityListModel(this);
    m_coins_list_model = new CoinsListModel(this);
    m_send_recipients = new SendRecipientsListModel(this);
    m_current_payment_request = new PaymentRequest(this);
}

WalletQmlModel::~WalletQmlModel()
{
    delete m_activity_list_model;
    delete m_coins_list_model;
    delete m_send_recipients;
    delete m_current_payment_request;
    if (m_current_transaction) {
        delete m_current_transaction;
    }
}

QString WalletQmlModel::balance() const
{
    if (!m_wallet) {
        return "0";
    }
    return QmlBitcoinUnits::format(QmlBitcoinUnits::Unit::BTC, m_wallet->getBalance());
}

CAmount WalletQmlModel::balanceSatoshi() const
{
    if (!m_wallet) {
        return 0;
    }
    return m_wallet->getBalance();
}

QString WalletQmlModel::name() const
{
    if (!m_wallet) {
        return QString();
    }
    return QString::fromStdString(m_wallet->getWalletName());
}

QString WalletQmlModel::newAddress(QString label)
{
    if (!m_wallet) {
        return QString();
    }
    OutputType output_type = m_wallet->getDefaultAddressType();
    util::Result<CTxDestination> dest{m_wallet->getNewDestination(output_type, label.toStdString())};
    return QString::fromStdString(EncodeDestination(dest.value()));
}

void WalletQmlModel::commitPaymentRequest()
{
    if (!m_wallet || !m_current_payment_request) {
        return;
    }

    if (m_current_payment_request->id().isEmpty()) {
        m_current_payment_request->setId(nextPaymentRequestId());
    }

    if (m_current_payment_request->address().isEmpty()) {
        const OutputType output_type = m_wallet->getDefaultAddressType();
        const auto destination{m_wallet->getNewDestination(output_type, m_current_payment_request->label().toStdString())};
        if (!destination) {
            return;
        }
        m_current_payment_request->setDestination(destination.value());
    }

    bool parse_ok{false};
    const int64_t request_id{m_current_payment_request->id().toLongLong(&parse_ok)};
    if (!parse_ok || request_id <= 0) {
        return;
    }

    QmlRecentRequestEntry request_entry;
    request_entry.id = request_id;
    request_entry.date = QDateTime::currentDateTime();
    request_entry.recipient.address = m_current_payment_request->address().toStdString();
    request_entry.recipient.label = m_current_payment_request->label().toStdString();
    request_entry.recipient.amount = m_current_payment_request->amount()->satoshi();
    request_entry.recipient.message = m_current_payment_request->message().toStdString();

    DataStream ss{};
    ss << request_entry;

    m_wallet->setAddressReceiveRequest(
        m_current_payment_request->destination(),
        m_current_payment_request->id().toStdString(),
        ss.str());
}

unsigned int WalletQmlModel::nextPaymentRequestId() const
{
    if (!m_wallet) {
        return 1;
    }

    int64_t max_id{0};
    for (const std::string& request : m_wallet->getAddressReceiveRequests()) {
        std::vector<uint8_t> data(request.begin(), request.end());
        DataStream ss{data};
        QmlRecentRequestEntry entry;
        try {
            ss >> entry;
        } catch (const std::ios_base::failure&) {
            continue;
        }
        if (entry.id > max_id) {
            max_id = entry.id;
        }
    }

    if (max_id <= 0 || max_id >= std::numeric_limits<unsigned int>::max() - 1) {
        return 1;
    }

    return static_cast<unsigned int>(max_id + 1);
}

std::set<interfaces::WalletTx> WalletQmlModel::getWalletTxs() const
{
    if (!m_wallet) {
        return {};
    }
    return m_wallet->getWalletTxs();
}

interfaces::WalletTx WalletQmlModel::getWalletTx(const uint256& hash) const
{
    if (!m_wallet) {
        return {};
    }
    return m_wallet->getWalletTx(Txid::FromUint256(hash));
}

bool WalletQmlModel::tryGetTxStatus(const uint256& txid,
                                    interfaces::WalletTxStatus& tx_status,
                                    int& num_blocks,
                                    int64_t& block_time) const
{
    if (!m_wallet) {
        return false;
    }
    return m_wallet->tryGetTxStatus(Txid::FromUint256(txid), tx_status, num_blocks, block_time);
}

QString WalletQmlModel::getAddressLabel(const QString& address) const
{
    if (!m_wallet || address.isEmpty()) {
        return {};
    }

    const CTxDestination destination = DecodeDestination(address.toStdString());
    if (!IsValidDestination(destination)) {
        return {};
    }

    std::string label;
    if (!m_wallet->getAddress(destination, &label, nullptr, nullptr)) {
        return {};
    }

    return QString::fromStdString(label);
}

std::unique_ptr<interfaces::Handler> WalletQmlModel::handleTransactionChanged(TransactionChangedFn fn)
{
    if (!m_wallet) {
        return nullptr;
    }
    return m_wallet->handleTransactionChanged(fn);
}

bool WalletQmlModel::prepareTransaction()
{
    if (!m_wallet || !m_send_recipients || m_send_recipients->recipients().empty()) {
        return false;
    }

    std::vector<wallet::CRecipient> vecSend;
    CAmount total = 0;
    for (auto* recipient : m_send_recipients->recipients()) {
        CTxDestination destination = DecodeDestination(recipient->address()->address().toStdString());
        wallet::CRecipient c_recipient = {destination, recipient->cAmount(), recipient->subtractFeeFromAmount()};
        m_coin_control.m_feerate = CFeeRate(1000);
        vecSend.push_back(c_recipient);
        total += recipient->cAmount();
    }

    CAmount balance = m_wallet->getBalance();
    if (balance < total) {
        return false;
    }

    int nChangePosRet = -1;
    CAmount nFeeRequired = 0;
    const auto& res = m_wallet->createTransaction(vecSend, m_coin_control, true, nChangePosRet, nFeeRequired);
    if (res) {
        if (m_current_transaction) {
            delete m_current_transaction;
        }
        CTransactionRef newTx = *res;
        m_current_transaction = new WalletQmlModelTransaction(m_send_recipients, this);
        m_current_transaction->setWtx(newTx);
        m_current_transaction->setTransactionFee(nFeeRequired);
        Q_EMIT currentTransactionChanged();
        return true;
    } else {
        return false;
    }
}

void WalletQmlModel::sendTransaction()
{
    if (!m_wallet || !m_current_transaction) {
        return;
    }

    CTransactionRef newTx = m_current_transaction->getWtx();
    if (!newTx) {
        return;
    }

    interfaces::WalletValueMap value_map;
    interfaces::WalletOrderForm order_form;
    m_wallet->commitTransaction(newTx, value_map, order_form);
}


void WalletQmlModel::setImportedPsbtError(const QString& error)
{
    m_imported_psbt.reset();
    m_imported_psbt_mode.clear();
    m_imported_psbt_status.clear();
    m_imported_psbt_error = error;
    m_imported_psbt_summary.clear();
    m_imported_psbt_can_sign = false;
    m_imported_psbt_can_broadcast = false;
    m_imported_psbt_complete = false;
    m_imported_psbt_unsigned_inputs = 0;
    m_imported_psbt_could_sign_inputs = 0;
    Q_EMIT importedPsbtChanged();
}

void WalletQmlModel::clearImportedPsbt()
{
    m_imported_psbt.reset();
    m_imported_psbt_mode.clear();
    m_imported_psbt_status.clear();
    m_imported_psbt_error.clear();
    m_imported_psbt_summary.clear();
    m_imported_psbt_can_sign = false;
    m_imported_psbt_can_broadcast = false;
    m_imported_psbt_complete = false;
    m_imported_psbt_unsigned_inputs = 0;
    m_imported_psbt_could_sign_inputs = 0;
    Q_EMIT importedPsbtChanged();
}

QString WalletQmlModel::importPsbtFromFile(const QString& path)
{
    const QString file_path{LocalFilePath(path)};
    QFile file(file_path);
    if (!file.open(QIODevice::ReadOnly)) {
        setImportedPsbtError(tr("Could not open PSBT file: %1").arg(file.errorString()));
        return QStringLiteral("unsupported");
    }

    const QByteArray bytes{file.readAll()};
    PartiallySignedTransaction psbt;
    std::string error;
    if (!DecodePsbtFromBytes(bytes, psbt, error)) {
        psbt = PartiallySignedTransaction{};
        std::string base64_error;
        if (!DecodeBase64PSBT(psbt, QString::fromUtf8(bytes).trimmed().toStdString(), base64_error)) {
            setImportedPsbtError(tr("Could not decode PSBT: %1").arg(QString::fromStdString(base64_error.empty() ? error : base64_error)));
            return QStringLiteral("unsupported");
        }
    }

    return importPsbt(std::move(psbt));
}

QString WalletQmlModel::importPsbt(PartiallySignedTransaction psbt)
{
    QString mode;
    QString reason;
    if (tryImportPsbtToReview(psbt, mode, reason)) {
        clearImportedPsbt();
        return mode;
    }

    setImportedPsbtError(reason.isEmpty() ? tr("This PSBT is not supported yet.") : reason);
    return QStringLiteral("unsupported");
}

bool WalletQmlModel::tryImportPsbtToReview(const PartiallySignedTransaction& psbt, QString& mode, QString& reason)
{
    if (!m_wallet) {
        reason = tr("No wallet is loaded.");
        return false;
    }
    if (!psbt.tx) {
        reason = tr("The PSBT does not contain an unsigned transaction.");
        return false;
    }
    if (psbt.tx->vin.empty() || psbt.tx->vout.empty()) {
        reason = tr("The PSBT has no inputs or outputs.");
        return false;
    }
    if (psbt.inputs.size() != psbt.tx->vin.size() || psbt.outputs.size() != psbt.tx->vout.size()) {
        reason = tr("The PSBT is malformed.");
        return false;
    }

    for (size_t i{0}; i < psbt.inputs.size(); ++i) {
        if (!m_wallet->txinIsMine(psbt.tx->vin[i])) {
            reason = tr("Only PSBTs that spend this wallet's own inputs are supported right now.");
            return false;
        }
    }

    PartiallySignedTransaction signed_psbt{psbt};
    bool complete{false};
    size_t signed_inputs{0};
    const std::optional<common::PSBTError> fill_error{
        m_wallet->fillPSBT(std::nullopt, /*sign=*/true, /*bip32derivs=*/true, &signed_inputs, signed_psbt, complete)};

    for (size_t i{0}; i < signed_psbt.inputs.size(); ++i) {
        if (IsMultisigPsbtInput(signed_psbt, i)) {
            reason = tr("Multisignature PSBTs are not supported yet.");
            return false;
        }
    }

    if (fill_error || !complete) {
        reason = tr("Only PSBTs this wallet can fully sign are supported right now.");
        return false;
    }

    struct DraftRecipient {
        QString address;
        QString label;
        CAmount amount;
    };
    std::vector<DraftRecipient> draft_recipients;
    CAmount recipient_total{0};
    for (const CTxOut& output : signed_psbt.tx->vout) {
        CTxDestination destination;
        if (!ExtractDestination(output.scriptPubKey, destination)) {
            reason = tr("Only PSBTs with standard address outputs are supported right now.");
            return false;
        }
        if (m_wallet->txoutIsMine(output)) {
            continue;
        }
        const QString address{QString::fromStdString(EncodeDestination(destination))};
        draft_recipients.push_back({address, getAddressLabel(address), output.nValue});
        recipient_total += output.nValue;
    }

    if (draft_recipients.empty()) {
        reason = tr("The PSBT does not have any recipient outputs to review.");
        return false;
    }
    if (draft_recipients.size() > 25) {
        reason = tr("The PSBT has more recipients than this send flow supports.");
        return false;
    }
    if (recipient_total <= 0) {
        reason = tr("The PSBT does not send a positive amount.");
        return false;
    }

    const node::PSBTAnalysis analysis{node::AnalyzePSBT(signed_psbt)};
    CMutableTransaction mutable_tx;
    if (!FinalizeAndExtractPSBT(signed_psbt, mutable_tx)) {
        reason = tr("Only PSBTs this wallet can fully sign are supported right now.");
        return false;
    }

    m_send_recipients->clear();
    for (size_t i{0}; i < draft_recipients.size(); ++i) {
        if (i > 0) {
            m_send_recipients->add();
        }
        SendRecipient* recipient{m_send_recipients->currentRecipient()};
        recipient->setAddress(draft_recipients[i].address);
        recipient->setLabel(draft_recipients[i].label);
        recipient->amount()->setSatoshi(draft_recipients[i].amount);
        recipient->setMessage(QString());
    }
    m_send_recipients->setCurrentIndex(0);

    if (m_current_transaction) {
        delete m_current_transaction;
    }
    m_current_transaction = new WalletQmlModelTransaction(m_send_recipients, this);
    m_current_transaction->setWtx(MakeTransactionRef(std::move(mutable_tx)));
    if (analysis.fee) {
        m_current_transaction->setTransactionFee(*analysis.fee);
    }
    Q_EMIT currentTransactionChanged();

    mode = draft_recipients.size() > 1 ? QStringLiteral("multiple-review") : QStringLiteral("single-review");
    return true;
}

QStringList WalletQmlModel::buildPsbtSummary(const PartiallySignedTransaction& psbt) const
{
    QStringList lines;
    if (!psbt.tx) {
        lines << tr("PSBT does not contain an unsigned transaction.");
        return lines;
    }

    CAmount total{0};
    for (const CTxOut& output : psbt.tx->vout) {
        total += output.nValue;
        CTxDestination destination;
        const QString address{ExtractDestination(output.scriptPubKey, destination) ? QString::fromStdString(EncodeDestination(destination)) : tr("unknown destination")};
        const bool own_address{m_wallet && m_wallet->txoutIsMine(output)};
        lines << tr("Sends %1 to %2%3").arg(FormatBtc(output.nValue), address, own_address ? tr(" (own address)") : QString());
    }

    const node::PSBTAnalysis analysis{node::AnalyzePSBT(psbt)};
    if (!analysis.error.empty()) {
        lines << tr("Analysis: %1").arg(QString::fromStdString(analysis.error));
    }
    if (analysis.fee) {
        lines << tr("Pays transaction fee: %1").arg(FormatBtc(*analysis.fee));
    } else {
        lines << tr("Transaction fee is not available.");
    }
    lines << tr("Total output amount: %1").arg(FormatBtc(total));

    const int unsigned_inputs{static_cast<int>(CountPSBTUnsignedInputs(psbt))};
    if (unsigned_inputs > 0) {
        lines << tr("Unsigned inputs: %1").arg(unsigned_inputs);
    }
    return lines;
}

void WalletQmlModel::refreshImportedPsbtState(const QString& status_override)
{
    if (!m_imported_psbt) {
        return;
    }

    bool complete{FinalizePSBT(*m_imported_psbt)};
    size_t could_sign{0};
    std::optional<common::PSBTError> fill_error;
    if (m_wallet) {
        fill_error = m_wallet->fillPSBT(std::nullopt, /*sign=*/false, /*bip32derivs=*/true, &could_sign, *m_imported_psbt, complete);
    }

    m_imported_psbt_error = fill_error ? PsbtErrorText(*fill_error) : QString();
    m_imported_psbt_complete = complete;
    m_imported_psbt_can_broadcast = complete;
    m_imported_psbt_could_sign_inputs = static_cast<int>(could_sign);
    m_imported_psbt_unsigned_inputs = static_cast<int>(CountPSBTUnsignedInputs(*m_imported_psbt));
    m_imported_psbt_can_sign = !complete && m_wallet && !m_wallet->privateKeysDisabled() && could_sign > 0;
    m_imported_psbt_summary = buildPsbtSummary(*m_imported_psbt);

    if (!status_override.isEmpty()) {
        m_imported_psbt_status = status_override;
    } else if (!m_imported_psbt_error.isEmpty()) {
        m_imported_psbt_status = m_imported_psbt_error;
    } else if (complete) {
        m_imported_psbt_status = tr("Transaction is fully signed and ready for broadcast.");
    } else if (!m_wallet) {
        m_imported_psbt_status = tr("No wallet is loaded. You can inspect, copy, or save this PSBT.");
    } else if (m_wallet->privateKeysDisabled()) {
        m_imported_psbt_status = tr("This wallet cannot sign transactions because private keys are disabled.");
    } else if (could_sign > 0) {
        m_imported_psbt_status = tr("This wallet can sign %1 input(s).").arg(static_cast<int>(could_sign));
    } else {
        m_imported_psbt_status = tr("This wallet does not have the right keys to sign this PSBT.");
    }

    Q_EMIT importedPsbtChanged();
}

void WalletQmlModel::signImportedPsbt()
{
    if (!m_imported_psbt || !m_wallet) {
        setImportedPsbtError(tr("No signable PSBT is loaded."));
        return;
    }
    if (m_wallet->privateKeysDisabled()) {
        refreshImportedPsbtState(tr("This wallet cannot sign transactions because private keys are disabled."));
        return;
    }

    bool complete{false};
    size_t signed_inputs{0};
    const std::optional<common::PSBTError> error{m_wallet->fillPSBT(std::nullopt, /*sign=*/true, /*bip32derivs=*/true, &signed_inputs, *m_imported_psbt, complete)};
    if (error) {
        refreshImportedPsbtState(tr("Could not sign PSBT: %1").arg(PsbtErrorText(*error)));
        return;
    }

    if (complete) {
        refreshImportedPsbtState(tr("PSBT signed. Transaction is ready for broadcast."));
    } else if (signed_inputs > 0) {
        refreshImportedPsbtState(tr("Signed %1 input(s). More signatures are still required.").arg(static_cast<int>(signed_inputs)));
    } else {
        refreshImportedPsbtState(tr("This wallet could not add any signatures to the PSBT."));
    }
}

void WalletQmlModel::broadcastImportedPsbt()
{
    if (!m_imported_psbt) {
        setImportedPsbtError(tr("No PSBT is loaded."));
        return;
    }
    if (!m_node) {
        refreshImportedPsbtState(tr("Cannot broadcast from this context because the node interface is unavailable."));
        return;
    }

    CMutableTransaction mutable_tx;
    if (!FinalizeAndExtractPSBT(*m_imported_psbt, mutable_tx)) {
        refreshImportedPsbtState(tr("PSBT is not complete and cannot be broadcast."));
        return;
    }

    const CTransactionRef tx{MakeTransactionRef(std::move(mutable_tx))};
    std::string error_string;
    const node::TransactionError error{m_node->broadcastTransaction(tx, node::DEFAULT_MAX_RAW_TX_FEE_RATE.GetFeePerK(), error_string)};
    if (error == node::TransactionError::OK) {
        refreshImportedPsbtState(tr("Transaction broadcast successfully. Transaction ID: %1").arg(QString::fromStdString(tx->GetHash().ToString())));
    } else {
        QString message{TransactionErrorText(error)};
        if (!error_string.empty()) {
            message += QStringLiteral(": ") + QString::fromStdString(error_string);
        }
        refreshImportedPsbtState(tr("Transaction broadcast failed: %1").arg(message));
    }
}

void WalletQmlModel::copyImportedPsbtToClipboard()
{
    if (!m_imported_psbt) {
        return;
    }
    QGuiApplication::clipboard()->setText(SerializePsbtBase64(*m_imported_psbt));
    refreshImportedPsbtState(tr("PSBT copied to clipboard."));
}

void WalletQmlModel::saveImportedPsbtToFile(const QString& path)
{
    if (!m_imported_psbt) {
        return;
    }
    QFile file(LocalFilePath(path));
    if (!file.open(QIODevice::WriteOnly)) {
        refreshImportedPsbtState(tr("Could not save PSBT: %1").arg(file.errorString()));
        return;
    }
    const QByteArray raw{SerializePsbtRaw(*m_imported_psbt)};
    if (file.write(raw) != raw.size()) {
        refreshImportedPsbtState(tr("Could not write the complete PSBT file."));
        return;
    }
    refreshImportedPsbtState(tr("PSBT saved."));
}

interfaces::Wallet::CoinsList WalletQmlModel::listCoins() const
{
    if (!m_wallet) {
        return {};
    }
    return m_wallet->listCoins();
}

bool WalletQmlModel::lockCoin(const COutPoint& output)
{
    if (!m_wallet) {
        return false;
    }
    return m_wallet->lockCoin(output, true);
}

bool WalletQmlModel::unlockCoin(const COutPoint& output)
{
    if (!m_wallet) {
        return false;
    }
    return m_wallet->unlockCoin(output);
}

bool WalletQmlModel::isLockedCoin(const COutPoint& output)
{
    if (!m_wallet) {
        return false;
    }
    return m_wallet->isLockedCoin(output);
}

void WalletQmlModel::listLockedCoins(std::vector<COutPoint>& outputs)
{
    if (!m_wallet) {
        return;
    }
    m_wallet->listLockedCoins(outputs);
}

void WalletQmlModel::selectCoin(const COutPoint& output)
{
    m_coin_control.Select(output);
}

void WalletQmlModel::unselectCoin(const COutPoint& output)
{
    m_coin_control.UnSelect(output);
}

bool WalletQmlModel::isSelectedCoin(const COutPoint& output)
{
    return m_coin_control.IsSelected(output);
}

std::vector<COutPoint> WalletQmlModel::listSelectedCoins() const
{
    return m_coin_control.ListSelected();
}

unsigned int WalletQmlModel::feeTargetBlocks() const
{
    return m_coin_control.m_confirm_target.value_or(wallet::DEFAULT_TX_CONFIRM_TARGET);
}

void WalletQmlModel::setFeeTargetBlocks(unsigned int target_blocks)
{
    if (m_coin_control.m_confirm_target != target_blocks) {
        m_coin_control.m_confirm_target = target_blocks;
        Q_EMIT feeTargetBlocksChanged();
    }
}
