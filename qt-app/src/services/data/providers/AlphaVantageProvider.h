#pragma once

#include <QByteArray>
#include <QFuture>
#include <QNetworkAccessManager>
#include <QSettings>
#include <QUrl>

#include "../RateLimiter.h"
#include "IMarketDataProvider.h"

class AlphaVantageProvider : public IMarketDataProvider
{
public:
    explicit AlphaVantageProvider(QNetworkAccessManager* manager = nullptr);
    ~AlphaVantageProvider() override;

    QFuture<Asset> fetchAsset(const QString& ticker) override;
    QFuture<QVector<double>> fetchHistoricalPrices(const QString& ticker, const QDate& from, const QDate& to) override;
    QString providerName() const override;

private:
    QString apiKey() const;
    QUrl makeUrl(const QString& functionName, const QString& ticker) const;
    QByteArray performGet(const QUrl& url) const;

    QNetworkAccessManager* m_networkAccessManager;
    bool m_ownsNetworkAccessManager;
    mutable RateLimiter m_rateLimiter;
};