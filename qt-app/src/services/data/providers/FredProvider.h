#pragma once

#include "../DataStore.h"
#include "IMarketDataProvider.h"
#include "ProviderBase.h"

class FredProvider : public IMarketDataProvider, protected ProviderBase
{
public:
    FredProvider();

    QFuture<Asset> fetchAsset(const QString& ticker) override;
    QFuture<QVector<double>> fetchHistoricalPrices(const QString& ticker, const QDate& from, const QDate& to) override;
    QFuture<double> fetchRiskFreeRate();
    QString providerName() const override;

private:
    QString apiKey() const;
    QUrl makeLatestRateUrl() const;
    QUrl makeHistoricalUrl(const QDate& from, const QDate& to) const;
    QString latestRateCacheKey() const;
    QString historicalCacheKey(const QDate& from, const QDate& to) const;

    DataStore m_dataStore;
};
