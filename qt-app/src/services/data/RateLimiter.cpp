#include "RateLimiter.h"

#include <QMutexLocker>
#include <QThread>
#include <QtConcurrent>

RateLimiter::RateLimiter(int tokensPerInterval, int intervalMs)
    : m_tokensPerInterval(tokensPerInterval)
    , m_intervalMs(intervalMs)
    , m_availableTokens(tokensPerInterval)
    , m_lastRefill(QDateTime::currentDateTimeUtc())
{
}

bool RateLimiter::tryAcquire()
{
    QMutexLocker locker(&m_mutex);
    refillIfNeeded();
    if (m_availableTokens <= 0) {
        return false;
    }

    --m_availableTokens;
    return true;
}

QFuture<void> RateLimiter::acquire()
{
    return QtConcurrent::run([this]() {
        while (!tryAcquire()) {
            QThread::msleep(100);
        }
    });
}

void RateLimiter::refillIfNeeded()
{
    const auto now = QDateTime::currentDateTimeUtc();
    const qint64 elapsed = m_lastRefill.msecsTo(now);
    if (elapsed < m_intervalMs) {
        return;
    }

    const int intervals = static_cast<int>(elapsed / m_intervalMs);
    m_availableTokens = qMin(m_tokensPerInterval, m_availableTokens + intervals * m_tokensPerInterval);
    m_lastRefill = m_lastRefill.addMSecs(intervals * m_intervalMs);
}