#include "FmpProvider.h"

#include <algorithm>

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

double jsonToDouble(const QJsonValue& value)
{
    if (value.isDouble()) {
        return value.toDouble();
    }

    return value.toString().toDouble();
}
}

FmpProvider::FmpProvider()
    : ProviderBase(250, 86400000)
{
}

QFuture<Asset> FmpProvider::fetchAsset(const QString& ticker)
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
            payload = performSecureGet(makeProfileUrl(asset.ticker));
            if (!payload.isEmpty()) {
                m_dataStore.saveCachedResponse(cacheKey, payload, kFundamentalCacheTtlSeconds);
            }
        }

        const QJsonArray profiles = QJsonDocument::fromJson(payload).array();
        if (profiles.isEmpty() || !profiles.first().isObject()) {
            return asset;
        }

        const QJsonObject json = profiles.first().toObject();
        const ValidationResult validation = ResponseValidator::validateFmpProfile(json);
        if (!validation.valid) {
            return asset;
        }

        asset.name = json.value(QStringLiteral("companyName")).toString();
        asset.sector = json.value(QStringLiteral("sector")).toString();
        asset.price = jsonToDouble(json.value(QStringLiteral("price")));
        asset.dividendYield = asset.price > 0.0 ? jsonToDouble(json.value(QStringLiteral("lastDiv"))) / asset.price : 0.0;
        asset.beta = jsonToDouble(json.value(QStringLiteral("beta")));
        asset.marketCap = jsonToDouble(json.value(QStringLiteral("mktCap")));
        asset.isETF = json.value(QStringLiteral("isEtf")).toBool();
        asset.lastUpdated = QDateTime::currentDateTimeUtc();
        return asset;
    });
}

QFuture<QVector<double>> FmpProvider::fetchHistoricalPrices(const QString& ticker, const QDate& from, const QDate& to)
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
        const QJsonArray historical = root.value(QStringLiteral("historical")).toArray();

        QVector<QPair<QDate, double>> datedPrices;
        datedPrices.reserve(historical.size());
        for (const QJsonValue& entry : historical) {
            const QJsonObject point = entry.toObject();
            const QDate date = QDate::fromString(point.value(QStringLiteral("date")).toString(), Qt::ISODate);
            if (!date.isValid()) {
                continue;
            }

            datedPrices.append({date, jsonToDouble(point.value(QStringLiteral("adjClose")))});
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

QString FmpProvider::providerName() const
{
    return QStringLiteral("Financial Modeling Prep");
}

QString FmpProvider::apiKey() const
{
    QSettings settings;
    return settings.value(QStringLiteral("marketData/fmpApiKey")).toString();
}

QUrl FmpProvider::makeProfileUrl(const QString& ticker) const
{
    QUrl url(QStringLiteral("https://financialmodelingprep.com/api/v3/profile/%1").arg(ticker));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("apikey"), apiKey());
    url.setQuery(query);
    return url;
}

QUrl FmpProvider::makeHistoricalUrl(const QString& ticker, const QDate& from, const QDate& to) const
{
    QUrl url(QStringLiteral("https://financialmodelingprep.com/api/v3/historical-price-full/%1").arg(ticker));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("from"), from.toString(Qt::ISODate));
    query.addQueryItem(QStringLiteral("to"), to.toString(Qt::ISODate));
    query.addQueryItem(QStringLiteral("apikey"), apiKey());
    url.setQuery(query);
    return url;
}

QString FmpProvider::assetCacheKey(const QString& ticker) const
{
    return QStringLiteral("fmp_asset_%1").arg(ticker);
}

QString FmpProvider::historicalCacheKey(const QString& ticker, const QDate& from, const QDate& to) const
{
    return QStringLiteral("fmp_history_%1_%2_%3")
        .arg(ticker, from.toString(Qt::ISODate), to.toString(Qt::ISODate));
}
