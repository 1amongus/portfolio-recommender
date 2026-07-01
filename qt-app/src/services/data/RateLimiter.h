#pragma once

#include <QDateTime>
#include <QFuture>
#include <QMutex>

class RateLimiter
{
public:
    RateLimiter(int tokensPerInterval = 1, int intervalMs = 1000);

    bool tryAcquire();
    QFuture<void> acquire();

private:
    void refillIfNeeded();

    int m_tokensPerInterval;
    int m_intervalMs;
    int m_availableTokens;
    QDateTime m_lastRefill;
    QMutex m_mutex;
};