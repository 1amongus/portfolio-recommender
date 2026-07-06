#include "FredProvider.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QSettings>
#include <QUrlQuery>
#include <QtConcurrent>

#include "ResponseValidator.h"

namespace {
constexpr int kPriceCacheTtlSeconds = 86400;

double observationToRate(const QJsonObject& observation)
{
    bool ok = false;
    const double value = observation.value(QStringLiteral("value")).toString().toDouble(&ok);
    return ok ? value / 100.0 : 0.0;
}
}

FredProvider::FredProvider()
    : ProviderBase(1000000, 60000)
{
}

QFuture<Asset> FredProvider::fetchAsset(const QString&)
{
    return QtConcurrent::run([this]() {
        Asset asset;
        asset.ticker = QStringLiteral("DGS10");
        const double rate = fetchRiskFreeRate().result();
        asset.name = QStringLiteral("10-Year Treasury Constant Maturity Rate");
        asset.sector = QStringLiteral("Macro");
        asset.price = rate;
        asset.dividendYield = rate;
        asset.beta = 0.0;
        asset.marketCap = 0.0;
        asset.isETF = false;
        asset.lastUpdated = QDateTime::currentDateTimeUtc();
        return asset;
    });
}

QFuture<QVector<double>> FredProvider::fetchHistoricalPrices(const QString&, const QDate& from, const QDate& to)
{
    return QtConcurrent::run([this, from, to]() {
        if (!from.isValid() || !to.isValid() || from > to || apiKey().trimmed().isEmpty()) {
            return QVector<double>{};
        }

        const QString cacheKey = historicalCacheKey(from, to);
        QByteArray payload = m_dataStore.loadCachedResponse(cacheKey);
        if (payload.isEmpty()) {
            payload = performSecureGet(makeHistoricalUrl(from, to));
            if (!payload.isEmpty()) {
                m_dataStore.saveCachedResponse(cacheKey, payload, kPriceCacheTtlSeconds);
            }
        }

        const QJsonObject root = QJsonDocument::fromJson(payload).object();
        const ValidationResult validation = ResponseValidator::validateFredObservations(root);
        if (!validation.valid) {
            return QVector<double>{};
        }

        const QJsonArray observations = root.value(QStringLiteral("observations")).toArray();
        QVector<double> rates;
        rates.reserve(observations.size());
        for (const QJsonValue& value : observations) {
            const QJsonObject observation = value.toObject();
            if (observation.value(QStringLiteral("value")).toString() == QStringLiteral(".")) {
                continue;
            }

            rates.append(observationToRate(observation));
        }

        return rates;
    });
}

QFuture<double> FredProvider::fetchRiskFreeRate()
{
    return QtConcurrent::run([this]() {
        if (apiKey().trimmed().isEmpty()) {
            return 0.0;
        }

        const QString cacheKey = latestRateCacheKey();
        QByteArray payload = m_dataStore.loadCachedResponse(cacheKey);
        if (payload.isEmpty()) {
            payload = performSecureGet(makeLatestRateUrl());
            if (!payload.isEmpty()) {
                m_dataStore.saveCachedResponse(cacheKey, payload, kPriceCacheTtlSeconds);
            }
        }

        const QJsonObject root = QJsonDocument::fromJson(payload).object();
        const ValidationResult validation = ResponseValidator::validateFredObservations(root);
        if (!validation.valid) {
            return 0.0;
        }

        const QJsonArray observations = root.value(QStringLiteral("observations")).toArray();
        if (observations.isEmpty() || !observations.first().isObject()) {
            return 0.0;
        }

        return observationToRate(observations.first().toObject());
    });
}

QString FredProvider::providerName() const
{
    return QStringLiteral("FRED");
}

QString FredProvider::apiKey() const
{
    QSettings settings;
    return settings.value(QStringLiteral("marketData/fredApiKey")).toString();
}

QUrl FredProvider::makeLatestRateUrl() const
{
    QUrl url(QStringLiteral("https://api.stlouisfed.org/fred/series/observations"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("series_id"), QStringLiteral("DGS10"));
    query.addQueryItem(QStringLiteral("api_key"), apiKey());
    query.addQueryItem(QStringLiteral("file_type"), QStringLiteral("json"));
    query.addQueryItem(QStringLiteral("sort_order"), QStringLiteral("desc"));
    query.addQueryItem(QStringLiteral("limit"), QStringLiteral("1"));
    url.setQuery(query);
    return url;
}

QUrl FredProvider::makeHistoricalUrl(const QDate& from, const QDate& to) const
{
    QUrl url(QStringLiteral("https://api.stlouisfed.org/fred/series/observations"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("series_id"), QStringLiteral("DGS10"));
    query.addQueryItem(QStringLiteral("api_key"), apiKey());
    query.addQueryItem(QStringLiteral("file_type"), QStringLiteral("json"));
    query.addQueryItem(QStringLiteral("sort_order"), QStringLiteral("asc"));
    query.addQueryItem(QStringLiteral("observation_start"), from.toString(Qt::ISODate));
    query.addQueryItem(QStringLiteral("observation_end"), to.toString(Qt::ISODate));
    url.setQuery(query);
    return url;
}

QString FredProvider::latestRateCacheKey() const
{
    return QStringLiteral("fred_dgs10_latest");
}

QString FredProvider::historicalCacheKey(const QDate& from, const QDate& to) const
{
    return QStringLiteral("fred_dgs10_%1_%2").arg(from.toString(Qt::ISODate), to.toString(Qt::ISODate));
}
