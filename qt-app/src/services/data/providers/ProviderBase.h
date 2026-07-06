#pragma once

#include <QByteArray>
#include <QNetworkAccessManager>
#include <QUrl>

#include <memory>

#include "../RateLimiter.h"

class ProviderBase
{
protected:
    ProviderBase(int rateLimit, int intervalMs);

    QByteArray performSecureGet(const QUrl& url) const;
    QNetworkAccessManager* networkManager() const;
    RateLimiter& rateLimiter();

private:
    std::unique_ptr<QNetworkAccessManager> m_networkManager;
    mutable RateLimiter m_rateLimiter;
};
