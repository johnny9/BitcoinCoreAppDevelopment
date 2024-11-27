// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qml/worker.h>

#include <QDebug>
#include <qobjectdefs.h>

Worker::Worker(QObject *parent)
    : QObject(parent)
    , m_stop_requested(false)
{
    moveToThread(&m_thread);

    connect(this, &Worker::workAvailable, this, &Worker::processWork, Qt::QueuedConnection);
    connect(&m_thread, &QThread::finished, this, &QObject::deleteLater);
}

Worker::~Worker()
{
}

void Worker::start()
{
    m_stop_requested = false;
    m_thread.start();
}

void Worker::stop()
{
    {
        QMutexLocker locker(&m_queue_mutex);
        m_stop_requested = true;
    }
    if (m_thread.isRunning()) {
        m_thread.quit();
    }
}

void Worker::doWork(Work task)
{
    {
        QMutexLocker locker(&m_queue_mutex);
        m_queue.enqueue(task);
    }
    Q_EMIT workAvailable();
}

void Worker::processWork()
{
    while (true) {
        Work work;
        {
            QMutexLocker locker(&m_queue_mutex);
            if (m_stop_requested) {
                return;
            }
            if (m_queue.isEmpty()) {
                break;
            }
            work = m_queue.dequeue();
        }

        if (work) {
            work();
        }
    }
}
