// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QML_WORKER_H
#define BITCOIN_QML_WORKER_H

#include <QObject>
#include <QThread>
#include <QQueue>
#include <QMutex>
#include <functional>

class Worker : public QObject {
    Q_OBJECT

public:
    using Work = std::function<void()>;

    explicit Worker(QObject *parent = nullptr);
    ~Worker();

    void doWork(Work task);
    void start();
    void stop();

Q_SIGNALS:
    void workAvailable();

public Q_SLOTS:
    void processWork();

private:
    QQueue<Work> m_queue;
    QMutex m_queue_mutex;
    QThread m_thread; 
    bool m_stop_requested;
};


#endif // BITCOIN_QML_WORKER_H