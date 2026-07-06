#pragma once

#include "../DataStore.h"
#include "IMarketDataProvider.h"
#include "ProviderBase.h"

class PolygonProvider : public IMarketDataProvider, protected ProviderBase
{
public:
    PolygonProvider();

    QFuture<Asset> fetchAsset(const QString& ticker) override;
    QFuture<QVector<double>> fetchHistoricalPrices(const QString& ticker, const QDate& from, const QDate& to) override;
    QString providerName() const override;

private:
    QString apiKey() const;
    QUrl makeAssetUrl(const QString& ticker) const;
    QUrl makeHistoricalUrl(const QString& ticker, const QDate& from, const QDate& to) const;
    QString assetCacheKey(const QString& ticker) const;
    QString historicalCacheKey(const QString& ticker, const QDate& from, const QDate& to) const;

    DataStore m_dataStore;
};
