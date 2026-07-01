#pragma once

#include <QDate>
#include <QFuture>
#include <QString>
#include <QVector>

#include "../../../models/Asset.h"

class IMarketDataProvider {
public:
    virtual ~IMarketDataProvider() = default;
    virtual QFuture<Asset> fetchAsset(const QString& ticker) = 0;
    virtual QFuture<QVector<double>> fetchHistoricalPrices(const QString& ticker, const QDate& from, const QDate& to) = 0;
    virtual QString providerName() const = 0;
};