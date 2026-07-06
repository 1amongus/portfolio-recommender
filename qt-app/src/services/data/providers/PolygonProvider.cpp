#include "PolygonProvider.h"

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
constexpr int kFundamentalCacheTtlSeconds = 604800;

double toDouble(const QJsonValue& value)
{
    if (value.isDouble()) {
        return value.toDouble();
    }

    return value.toString().toDouble();
}
}

PolygonProvider::PolygonProvider()
    : ProviderBase(5, 60000)
{
}

QFuture<Asset> PolygonProvider::fetchAsset(const QString& ticker)
{
    return QtConcurrent::run([this, ticker]() {
        Asset asset;
        asset.ticker = ticker.trimmed().toUpper();

        if (asset.ticker.isEmpty() || apiKey().trimmed().isEmpty()) {
            return asset;
        }

        const QString cacheKey = assetCacheKey(asset.ticker);
        QByteArray payload = m_dataStore.loadCachedResponse(cacheKey);
        if (payload.isEmpty()) {
            payload = performSecureGet(makeAssetUrl(asset.ticker));
            if (!payload.isEmpty()) {
                m_dataStore.saveCachedResponse(cacheKey, payload, kFundamentalCacheTtlSeconds);
            }
        }

        const QJsonObject root = QJsonDocument::fromJson(payload).object();
        const QJsonObject results = root.value(QStringLiteral("results")).toObject();
        if (results.isEmpty()) {
            return asset;
        }

        asset.name = results.value(QStringLiteral("name")).toString();
        asset.sector = results.value(QStringLiteral("sic_description")).toString();
        asset.price = 0.0;
        asset.dividendYield = 0.0;
        asset.beta = 0.0;
        asset.marketCap = toDouble(results.value(QStringLiteral("market_cap")));
        asset.isETF = results.value(QStringLiteral("type")).toString().compare(QStringLiteral("ETF"), Qt::CaseInsensitive) == 0;
        asset.lastUpdated = QDateTime::currentDateTimeUtc();
        return asset;
    });
}

QFuture<QVector<double>> PolygonProvider::fetchHistoricalPrices(const QString& ticker, const QDate& from, const QDate& to)
{
    return QtConcurrent::run([this, ticker, from, to]() {
        if (!from.isValid() || !to.isValid() || from > to || apiKey().trimmed().isEmpty()) {
            return QVector<double>{};
        }

        const QString normalizedTicker = ticker.trimmed().toUpper();
        const QString cacheKey = historicalCacheKey(normalizedTicker, from, to);
        QByteArray payload = m_dataStore.loadCachedResponse(cacheKey);
        if (payload.isEmpty()) {
            payload = performSecureGet(makeHistoricalUrl(normalizedTicker, from, to));
            if (!payload.isEmpty()) {
                m_dataStore.saveCachedResponse(cacheKey, payload, kPriceCacheTtlSeconds);
            }
        }

        const QJsonObject root = QJsonDocument::fromJson(payload).object();
        const ValidationResult validation = ResponseValidator::validatePolygonAggs(root);
        if (!validation.valid) {
            return QVector<double>{};
        }

        const QJsonArray results = root.value(QStringLiteral("results")).toArray();
        QVector<double> prices;
        prices.reserve(results.size());
        for (const QJsonValue& value : results) {
            prices.append(toDouble(value.toObject().value(QStringLiteral("c"))));
        }

        return prices;
    });
}

QString PolygonProvider::providerName() const
{
    return QStringLiteral("Polygon.io");
}

QString PolygonProvider::apiKey() const
{
    QSettings settings;
    return settings.value(QStringLiteral("marketData/polygonApiKey")).toString();
}

QUrl PolygonProvider::makeAssetUrl(const QString& ticker) const
{
    QUrl url(QStringLiteral("https://api.polygon.io/v3/reference/tickers/%1").arg(ticker));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("apiKey"), apiKey());
    url.setQuery(query);
    return url;
}

QUrl PolygonProvider::makeHistoricalUrl(const QString& ticker, const QDate& from, const QDate& to) const
{
    QUrl url(QStringLiteral("https://api.polygon.io/v2/aggs/ticker/%1/range/1/day/%2/%3")
                 .arg(ticker, from.toString(Qt::ISODate), to.toString(Qt::ISODate)));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("adjusted"), QStringLiteral("true"));
    query.addQueryItem(QStringLiteral("sort"), QStringLiteral("asc"));
    query.addQueryItem(QStringLiteral("apiKey"), apiKey());
    url.setQuery(query);
    return url;
}

QString PolygonProvider::assetCacheKey(const QString& ticker) const
{
    return QStringLiteral("polygon_asset_%1").arg(ticker);
}

QString PolygonProvider::historicalCacheKey(const QString& ticker, const QDate& from, const QDate& to) const
{
    return QStringLiteral("polygon_history_%1_%2_%3")
        .arg(ticker, from.toString(Qt::ISODate), to.toString(Qt::ISODate));
}
