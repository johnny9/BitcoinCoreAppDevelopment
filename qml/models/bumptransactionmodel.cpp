// Copyright (c) 2026 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qml/models/bumptransactionmodel.h>

#include <interfaces/wallet.h>
#include <qml/bitcoinamount.h>
#include <wallet/coincontrol.h>

namespace {
QString FormatFee(CAmount amount)
{
    BitcoinAmount bitcoin_amount;
    bitcoin_amount.setSatoshi(amount);
    return bitcoin_amount.toDisplay() + QStringLiteral(" ") + bitcoin_amount.unitLabel();
}
} // namespace

BumpTransactionModel::BumpTransactionModel(interfaces::Wallet* wallet, QObject* parent)
    : QObject(parent)
    , m_wallet(wallet)
{
}

QString BumpTransactionModel::oldFee() const
{
    return FormatFee(m_old_fee);
}

QString BumpTransactionModel::newFee() const
{
    return FormatFee(m_new_fee);
}

QString BumpTransactionModel::feeIncrease() const
{
    return FormatFee(m_new_fee - m_old_fee);
}

QString BumpTransactionModel::oldTxid() const
{
    return QString::fromStdString(m_original_txid.GetHex());
}

void BumpTransactionModel::setState(State state)
{
    if (m_state != state) {
        m_state = state;
        Q_EMIT stateChanged();
    }
}

void BumpTransactionModel::setActionType(ActionType type)
{
    if (m_action_type != type) {
        m_action_type = type;
        Q_EMIT actionTypeChanged();
    }
}

void BumpTransactionModel::setError(const QString& error)
{
    m_error = error;
    setState(Failed);
    Q_EMIT resultChanged();
}

void BumpTransactionModel::prepareFeeBump(const QString& txid, unsigned int targetBlocks)
{
    if (!m_wallet) {
        setError(tr("No wallet available."));
        return;
    }

    auto parsed = uint256::FromHex(txid.toStdString());
    if (!parsed) {
        setError(tr("Invalid transaction ID."));
        return;
    }

    setActionType(SpeedUp);
    setState(Preparing);

    wallet::CCoinControl coin_control;
    coin_control.m_signal_bip125_rbf = true;
    coin_control.m_confirm_target = targetBlocks;

    std::vector<bilingual_str> errors;
    CAmount old_fee{0};
    CAmount new_fee{0};
    CMutableTransaction mtx;

    if (!m_wallet->createBumpTransaction(Txid::FromUint256(*parsed), coin_control, errors, old_fee, new_fee, mtx)) {
        setError(errors.empty() ? tr("Failed to create bump transaction.") : QString::fromStdString(errors[0].translated));
        return;
    }

    m_original_txid = Txid::FromUint256(*parsed);
    m_old_fee = old_fee;
    m_new_fee = new_fee;
    m_bump_mtx = std::move(mtx);

    setState(NeedsConfirmation);
    Q_EMIT resultChanged();
}

void BumpTransactionModel::confirmFeeBump()
{
    if (!m_wallet || m_state != NeedsConfirmation) {
        return;
    }

    setState(Committing);

    if (!m_wallet->transactionCanBeBumped(m_original_txid)) {
        setError(tr("Transaction can no longer be bumped."));
        return;
    }

    if (!m_wallet->signBumpTransaction(m_bump_mtx)) {
        setError(tr("Failed to sign transaction."));
        return;
    }

    std::vector<bilingual_str> errors;
    Txid bumped_txid;
    if (!m_wallet->commitBumpTransaction(m_original_txid, std::move(m_bump_mtx), errors, bumped_txid)) {
        setError(errors.empty() ? tr("Failed to commit transaction.") : QString::fromStdString(errors[0].translated));
        return;
    }

    m_new_txid = QString::fromStdString(bumped_txid.GetHex());
    setState(Succeeded);
    Q_EMIT resultChanged();
}

void BumpTransactionModel::reset()
{
    m_state = Idle;
    m_action_type = SpeedUp;
    m_old_fee = 0;
    m_new_fee = 0;
    m_bump_mtx = CMutableTransaction{};
    m_original_txid = Txid{};
    m_new_txid.clear();
    m_error.clear();

    Q_EMIT stateChanged();
    Q_EMIT actionTypeChanged();
    Q_EMIT resultChanged();
}
