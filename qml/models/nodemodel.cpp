// Copyright (c) 2021 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qml/models/nodemodel.h>

#include <interfaces/node.h>
#include <net.h>
#include <net_processing.h>
#include <netbase.h>
#include <node/interface_ui.h>
#include <validation.h>

#include <cassert>
#include <chrono>

#include <QDateTime>
#include <QMetaObject>
#include <QString>
#include <QTimerEvent>

NodeModel::NodeModel(interfaces::Node& node)
    : m_node{node}
{
    ConnectToBlockTipSignal();
    ConnectToNumConnectionsChangedSignal();
    ConnectToBannedListChangedSignal();
    refreshMempoolInfo();
}

void NodeModel::setBlockTipHeight(int new_height)
{
    if (new_height != m_block_tip_height) {
        m_block_tip_height = new_height;
        Q_EMIT blockTipHeightChanged();
    }
}

void NodeModel::setNumOutboundPeers(int new_num)
{
    if (new_num != m_num_outbound_peers) {
        m_num_outbound_peers = new_num;
        Q_EMIT numOutboundPeersChanged();
    }
}

void NodeModel::refreshMempoolInfo()
{
    const int mempool_transaction_count = static_cast<int>(m_node.getMempoolSize());
    const double mempool_usage_mb = m_node.getMempoolDynamicUsage() / 1'000'000.0;
    const double mempool_max_usage_mb = m_node.getMempoolMaxUsage() / 1'000'000.0;

    if (mempool_transaction_count != m_mempool_transaction_count ||
        mempool_usage_mb != m_mempool_usage_mb ||
        mempool_max_usage_mb != m_mempool_max_usage_mb) {
        m_mempool_transaction_count = mempool_transaction_count;
        m_mempool_usage_mb = mempool_usage_mb;
        m_mempool_max_usage_mb = mempool_max_usage_mb;
        Q_EMIT mempoolInfoChanged();
    }
}

void NodeModel::setRemainingSyncTime(double new_progress)
{
    int currentTime = QDateTime::currentDateTime().toMSecsSinceEpoch();

    // keep a vector of samples of verification progress at height
    m_block_process_time.push_front(qMakePair(currentTime, new_progress));

    // show progress speed if we have more than one sample
    if (m_block_process_time.size() >= 2) {
        double progressDelta = 0;
        int timeDelta = 0;
        int remainingMSecs = 0;
        double remainingProgress = 1.0 - new_progress;
        for (int i = 1; i < m_block_process_time.size(); i++) {
            QPair<int, double> sample = m_block_process_time[i];

            // take first sample after 500 seconds or last available one
            if (sample.first < (currentTime - 500 * 1000) || i == m_block_process_time.size() - 1) {
                progressDelta = m_block_process_time[0].second - sample.second;
                timeDelta = m_block_process_time[0].first - sample.first;
                remainingMSecs = (progressDelta > 0) ? remainingProgress / progressDelta * timeDelta : -1;
                break;
            }
        }
        if (remainingMSecs > 0 && m_block_process_time.count() % 1000 == 0) {
            m_remaining_sync_time = remainingMSecs;

            Q_EMIT remainingSyncTimeChanged();
        }
        static const int MAX_SAMPLES = 5000;
        if (m_block_process_time.count() > MAX_SAMPLES) {
            m_block_process_time.remove(1, m_block_process_time.count() - 1);
        }
    }
}
void NodeModel::setVerificationProgress(double new_progress)
{
    if (new_progress != m_verification_progress) {
        setRemainingSyncTime(new_progress);

        m_verification_progress = new_progress;
        Q_EMIT verificationProgressChanged();
    }
}

void NodeModel::setPause(bool new_pause)
{
    if(m_pause != new_pause) {
        m_pause = new_pause;
        m_node.setNetworkActive(!new_pause);
        Q_EMIT pauseChanged(new_pause);
    }
}

void NodeModel::setErrorState(bool faulted)
{
    if (m_faulted != faulted) {
        m_faulted = faulted;
        Q_EMIT errorStateChanged(faulted);
    }
}

void NodeModel::startNodeInitializionThread()
{
    if (m_initialization_requested) {
        return;
    }
    m_initialization_requested = true;
    Q_EMIT requestedInitialize();
}

void NodeModel::requestShutdown()
{
    if (m_shutdown_requested) return;
    m_shutdown_requested = true;
    stopShutdownPolling();
    m_node.startShutdown();
    Q_EMIT requestedShutdown();
}

void NodeModel::initializeResult(bool success, interfaces::BlockAndHeaderTipInfo tip_info)
{
    if (!success) {
        setErrorState(true);
    } else {
        refreshMempoolInfo();
        setBlockTipHeight(tip_info.block_height);
        setVerificationProgress(tip_info.verification_progress);
        Q_EMIT setTimeRatioListInitial();
    }
    Q_EMIT nodeInitialized();
}

void NodeModel::startShutdownPolling()
{
    if (m_shutdown_polling_timer_id != 0) return;
    m_shutdown_polling_timer_id = startTimer(200ms);
}

void NodeModel::stopShutdownPolling()
{
    if (m_shutdown_polling_timer_id == 0) return;
    killTimer(m_shutdown_polling_timer_id);
    m_shutdown_polling_timer_id = 0;
}

void NodeModel::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == m_shutdown_polling_timer_id && m_node.shutdownRequested()) {
        requestShutdown();
    }
}

void NodeModel::ConnectToBlockTipSignal()
{
    assert(!m_handler_notify_block_tip);

    m_handler_notify_block_tip = m_node.handleNotifyBlockTip(
        [this]([[maybe_unused]] SynchronizationState state, interfaces::BlockTip tip, double verification_progress) {
            QMetaObject::invokeMethod(this, [this, block_height = tip.block_height, block_time = tip.block_time, verification_progress] {
                setBlockTipHeight(block_height);
                setVerificationProgress(verification_progress);

                Q_EMIT setTimeRatioList(block_time);
            });
        });
}

void NodeModel::ConnectToNumConnectionsChangedSignal()
{
    assert(!m_handler_notify_num_peers_changed);

    m_handler_notify_num_peers_changed = m_node.handleNotifyNumConnectionsChanged(
        [this](int new_num_connections) {
            QMetaObject::invokeMethod(this, [this, new_num_connections] {
                setNumOutboundPeers(new_num_connections);
            });
        });
}

bool NodeModel::validateProxyAddress(QString address_port)
{
    uint16_t port{0};
    std::string addr_port{address_port.toStdString()};
    std::string hostname;
    // First, attempt to split the input address into hostname and port components.
    // We call SplitHostPort to validate that a port is provided in addr_port.
    // If either splitting fails or port is zero (not specified), return false.
    if (!SplitHostPort(addr_port, port, hostname) || !port) return false;

    // Create a service endpoint (CService) from the address and port.
    // If port is missing in addr_port, DEFAULT_PROXY_PORT is used as the fallback.
    CService serv(LookupNumeric(addr_port, DEFAULT_PROXY_PORT));

    // Construct the Proxy with the service endpoint and return if it's valid
    Proxy addrProxy = Proxy(serv, true);
    return addrProxy.IsValid();
}

QString NodeModel::defaultProxyAddress()
{
    return QString::fromStdString(std::string(DEFAULT_PROXY_HOST) + ":" + util::ToString(DEFAULT_PROXY_PORT));
}

bool NodeModel::disconnectPeer(int nodeId)
{
    return m_node.disconnectById(nodeId);
}

bool NodeModel::banPeer(const QString& rawAddress, int64_t banDuration)
{
    auto addr = LookupHost(rawAddress.toStdString(), /*fAllowLookup=*/false);
    if (!addr) return false;
    bool result = m_node.ban(*addr, banDuration);
    if (result) {
        m_node.disconnectByAddress(*addr);
    }
    return result;
}

void NodeModel::ConnectToBannedListChangedSignal()
{
    assert(!m_handler_notify_banned_list_changed);
    m_handler_notify_banned_list_changed = m_node.handleBannedListChanged([this]() {
        QMetaObject::invokeMethod(this, [this] {
            Q_EMIT bannedListChanged();
        });
    });
}
