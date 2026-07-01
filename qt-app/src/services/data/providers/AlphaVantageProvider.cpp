#include "AlphaVantageProvider.h"

#include <algorithm>

#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QtConcurrent>

AlphaVantageProvider::AlphaVantageProvider(QNetworkAccessManager* manager)
    : m_networkAccessManager(manager != nullptr ? manager : new QNetworkAccessManager())
    , m_ownsNetworkAccessManager(manager == nullptr)
    , m_rateLimiter(5, 60000)
{
}

AlphaVantageProvider::~AlphaVantageProvider()
{
    if (m_ownsNetworkAccessManager) {
        delete m_networkAccessManager;
    }
}

QFuture<Asset> AlphaVantageProvider::fetchAsset(const QString& ticker)
{
    return QtConcurrent::run([this, ticker]() {
        m_rateLimiter.acquire().waitForFinished();
        const auto payload = performGet(makeUrl(QStringLiteral("OVERVIEW"), ticker));
        const auto json = QJsonDocument::fromJson(payload).object();

        Asset asset;
        asset.ticker = ticker.toUpper();
        asset.name = json.value(QStringLiteral("Name")).toString();
        asset.sector = json.value(QStringLiteral("Sector")).toString();
        asset.price = json.value(QStringLiteral("52WeekHigh")).toString().toDouble();
        asset.dividendYield = json.value(QStringLiteral("DividendYield")).toString().toDouble();
        asset.beta = json.value(QStringLiteral("Beta")).toString().toDouble();
        asset.marketCap = json.value(QStringLiteral("MarketCapitalization")).toString().toDouble();
        asset.isETF = json.value(QStringLiteral("AssetType")).toString().compare(QStringLiteral("ETF"), Qt::CaseInsensitive) == 0;
        asset.lastUpdated = QDateTime::currentDateTimeUtc();
        return asset;
    });
}

QFuture<QVector<double>> AlphaVantageProvider::fetchHistoricalPrices(const QString& ticker, const QDate& from, const QDate& to)
{
    return QtConcurrent::run([this, ticker, from, to]() {
        m_rateLimiter.acquire().waitForFinished();
        const auto payload = performGet(makeUrl(QStringLiteral("TIME_SERIES_DAILY_ADJUSTED"), ticker));
        const auto root = QJsonDocument::fromJson(payload).object();
        const auto series = root.value(QStringLiteral("Time Series (Daily)")).toObject();

        QVector<QPair<QDate, double>> datedPrices;
        const auto keys = series.keys();
        for (const auto& key : keys) {
            const auto date = QDate::fromString(key, Qt::ISODate);
            if (!date.isValid() || date < from || date > to) {
                continue;
            }

            const auto point = series.value(key).toObject();
            datedPrices.append({date, point.value(QStringLiteral("5. adjusted close")).toString().toDouble()});
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

QByteArray AlphaVantageProvider::performGet(const QUrl& url) const
{
    QNetworkRequest request(url);
    QEventLoop loop;
    QNetworkReply* reply = m_networkAccessManager->get(request);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    const QByteArray payload = reply->readAll();
    reply->deleteLater();
    return payload;
}