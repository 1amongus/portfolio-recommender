#include "ProviderBase.h"

#include <QEventLoop>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QThread>
#include <QtGlobal>

ProviderBase::ProviderBase(int rateLimit, int intervalMs)
    : m_networkManager(std::make_unique<QNetworkAccessManager>())
    , m_rateLimiter(rateLimit, intervalMs)
{
}

QByteArray ProviderBase::performSecureGet(const QUrl& url) const
{
    Q_ASSERT_X(url.scheme().compare(QStringLiteral("https"), Qt::CaseInsensitive) == 0,
               "ProviderBase::performSecureGet",
               "Only HTTPS endpoints are allowed.");
    if (url.scheme().compare(QStringLiteral("https"), Qt::CaseInsensitive) != 0) {
        return {};
    }

    m_rateLimiter.acquire().waitForFinished();

    QNetworkRequest request(url);
    QEventLoop loop;
    QNetworkAccessManager localManager;
    QNetworkAccessManager* manager = m_networkManager.get();
    if (manager == nullptr || manager->thread() != QThread::currentThread()) {
        manager = &localManager;
    }

    QNetworkReply* reply = manager->get(request);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    const QByteArray payload = reply->error() == QNetworkReply::NoError ? reply->readAll() : QByteArray{};
    reply->deleteLater();
    return payload;
}

QNetworkAccessManager* ProviderBase::networkManager() const
{
    return m_networkManager.get();
}

RateLimiter& ProviderBase::rateLimiter()
{
    return m_rateLimiter;
}
