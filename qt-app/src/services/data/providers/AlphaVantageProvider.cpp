#include "AlphaVantageProvider.h"

#include <algorithm>

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

double valueToDouble(const QJsonValue& value)
{
    if (value.isDouble()) {
        return value.toDouble();
    }

    return value.toString().toDouble();
}
}

AlphaVantageProvider::AlphaVantageProvider()
    : ProviderBase(5, 60000)
{
}

QFuture<Asset> AlphaVantageProvider::fetchAsset(const QString& ticker)
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
            payload = performSecureGet(makeUrl(QStringLiteral("OVERVIEW"), asset.ticker));
            if (!payload.isEmpty()) {
                m_dataStore.saveCachedResponse(cacheKey, payload, kFundamentalCacheTtlSeconds);
            }
        }

        const QJsonObject json = QJsonDocument::fromJson(payload).object();
        const ValidationResult validation = ResponseValidator::validateAlphaVantageOverview(json);
        if (!validation.valid) {
            return asset;
        }

        asset.name = json.value(QStringLiteral("Name")).toString();
        asset.sector = json.value(QStringLiteral("Sector")).toString();
        asset.price = valueToDouble(json.value(QStringLiteral("50DayMovingAverage")));
        asset.dividendYield = valueToDouble(json.value(QStringLiteral("DividendYield")));
        asset.beta = valueToDouble(json.value(QStringLiteral("Beta")));
        asset.marketCap = valueToDouble(json.value(QStringLiteral("MarketCapitalization")));
        asset.isETF = json.value(QStringLiteral("AssetType")).toString().compare(QStringLiteral("ETF"), Qt::CaseInsensitive) == 0;
        asset.lastUpdated = QDateTime::currentDateTimeUtc();
        return asset;
    });
}

QFuture<QVector<double>> AlphaVantageProvider::fetchHistoricalPrices(const QString& ticker, const QDate& from, const QDate& to)
{
    return QtConcurrent::run([this, ticker, from, to]() {
        if (!from.isValid() || !to.isValid() || from > to || apiKey().trimmed().isEmpty()) {
            return QVector<double>{};
        }

        const QString normalizedTicker = ticker.trimmed().toUpper();
        const QString cacheKey = historicalCacheKey(normalizedTicker, from, to);
        QByteArray payload = m_dataStore.loadCachedResponse(cacheKey);
        if (payload.isEmpty()) {
            payload = performSecureGet(makeUrl(QStringLiteral("TIME_SERIES_DAILY_ADJUSTED"), normalizedTicker));
            if (!payload.isEmpty()) {
                m_dataStore.saveCachedResponse(cacheKey, payload, kPriceCacheTtlSeconds);
            }
        }

        const QJsonObject root = QJsonDocument::fromJson(payload).object();
        const ValidationResult validation = ResponseValidator::validateAlphaVantageTimeSeries(root);
        if (!validation.valid) {
            return QVector<double>{};
        }

        const QJsonObject series = root.value(QStringLiteral("Time Series (Daily)")).toObject();

        QVector<QPair<QDate, double>> datedPrices;
        for (auto it = series.constBegin(); it != series.constEnd(); ++it) {
            const QDate date = QDate::fromString(it.key(), Qt::ISODate);
            if (!date.isValid() || date < from || date > to) {
                continue;
            }

            const QJsonObject point = it.value().toObject();
            datedPrices.append({date, valueToDouble(point.value(QStringLiteral("5. adjusted close")))});
        }

        std::sort(datedPrices.begin(), datedPrices.end(), [](const auto& left, const auto& right) {
            return left.first < right.first;
        });

        QVector<double> prices;
        prices.reserve(datedPrices.size());
        for (const auto& entry : datedPrices) {
            prices.append(entry.second);
        }

        return prices;
    });
}

QString AlphaVantageProvider::providerName() const
{
    return QStringLiteral("Alpha Vantage");
}

QString AlphaVantageProvider::apiKey() const
{
    QSettings settings;
    return settings.value(QStringLiteral("marketData/alphaVantageApiKey")).toString();
}

QUrl AlphaVantageProvider::makeUrl(const QString& functionName, const QString& ticker) const
{
    QUrl url(QStringLiteral("https://www.alphavantage.co/query"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("function"), functionName);
    query.addQueryItem(QStringLiteral("symbol"), ticker);
    query.addQueryItem(QStringLiteral("apikey"), apiKey());
    query.addQueryItem(QStringLiteral("outputsize"), QStringLiteral("full"));
    url.setQuery(query);
    return url;
}

QString AlphaVantageProvider::assetCacheKey(const QString& ticker) const
{
    return QStringLiteral("alphavantage_asset_%1").arg(ticker);
}

QString AlphaVantageProvider::historicalCacheKey(const QString& ticker, const QDate& from, const QDate& to) const
{
    return QStringLiteral("alphavantage_history_%1_%2_%3")
        .arg(ticker, from.toString(Qt::ISODate), to.toString(Qt::ISODate));
}
