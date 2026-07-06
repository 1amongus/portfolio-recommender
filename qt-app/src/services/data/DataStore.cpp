#include "DataStore.h"



#include <algorithm>



#include <QDateTime>

#include <QDir>

#include <QFile>

#include <QJsonArray>

#include <QJsonDocument>

#include <QJsonObject>

#include <QRegularExpression>

#include <QStandardPaths>



namespace {

bool atomicWriteJson(const QString& path, const QJsonDocument& document);

QJsonObject holdingToJson(const Holding& holding)

{

    return {

        {QStringLiteral("ticker"), holding.ticker},

        {QStringLiteral("weight"), holding.weight},

        {QStringLiteral("yield"), holding.yield},

        {QStringLiteral("beta"), holding.beta}

    };

}



Holding holdingFromJson(const QJsonObject& object)

{

    Holding holding;

    holding.ticker = object.value(QStringLiteral("ticker")).toString();

    holding.weight = object.value(QStringLiteral("weight")).toDouble();

    holding.yield = object.value(QStringLiteral("yield")).toDouble();

    holding.beta = object.value(QStringLiteral("beta")).toDouble();

    return holding;

}



QJsonObject portfolioToJson(const Portfolio& portfolio)

{

    QJsonArray holdings;

    for (const auto& holding : portfolio.holdings) {

        holdings.append(holdingToJson(holding));

    }



    return {

        {QStringLiteral("id"), portfolio.id},

        {QStringLiteral("name"), portfolio.name},

        {QStringLiteral("targetYield"), portfolio.targetYield},

        {QStringLiteral("aggregateBeta"), portfolio.aggregateBeta},

        {QStringLiteral("achievedYield"), portfolio.achievedYield},

        {QStringLiteral("holdings"), holdings},

        {QStringLiteral("createdAt"), portfolio.createdAt.toString(Qt::ISODate)}

    };

}



Portfolio portfolioFromJson(const QJsonObject& object)

{

    Portfolio portfolio;

    portfolio.id = object.value(QStringLiteral("id")).toString();

    portfolio.name = object.value(QStringLiteral("name")).toString();

    portfolio.targetYield = object.value(QStringLiteral("targetYield")).toDouble();

    portfolio.aggregateBeta = object.value(QStringLiteral("aggregateBeta")).toDouble();

    portfolio.achievedYield = object.value(QStringLiteral("achievedYield")).toDouble();

    portfolio.createdAt = QDateTime::fromString(object.value(QStringLiteral("createdAt")).toString(), Qt::ISODate);



    const auto holdings = object.value(QStringLiteral("holdings")).toArray();

    for (const auto& value : holdings) {

        portfolio.holdings.append(holdingFromJson(value.toObject()));

    }



    return portfolio;

}

bool writePortfolios(const QString& path, const QVector<Portfolio>& portfolios)

{

    QJsonArray items;

    for (const auto& item : portfolios) {

        items.append(portfolioToJson(item));

    }



    return atomicWriteJson(path, QJsonDocument(items));

}



QJsonObject assetToJson(const Asset& asset)

{

    return {

        {QStringLiteral("ticker"), asset.ticker},

        {QStringLiteral("name"), asset.name},

        {QStringLiteral("sector"), asset.sector},

        {QStringLiteral("price"), asset.price},

        {QStringLiteral("dividendYield"), asset.dividendYield},

        {QStringLiteral("beta"), asset.beta},

        {QStringLiteral("marketCap"), asset.marketCap},

        {QStringLiteral("isETF"), asset.isETF},

        {QStringLiteral("lastUpdated"), asset.lastUpdated.toString(Qt::ISODate)}

    };

}



Asset assetFromJson(const QJsonObject& object)

{

    Asset asset;

    asset.ticker = object.value(QStringLiteral("ticker")).toString();

    asset.name = object.value(QStringLiteral("name")).toString();

    asset.sector = object.value(QStringLiteral("sector")).toString();

    asset.price = object.value(QStringLiteral("price")).toDouble();

    asset.dividendYield = object.value(QStringLiteral("dividendYield")).toDouble();

    asset.beta = object.value(QStringLiteral("beta")).toDouble();

    asset.marketCap = object.value(QStringLiteral("marketCap")).toDouble();

    asset.isETF = object.value(QStringLiteral("isETF")).toBool();

    asset.lastUpdated = QDateTime::fromString(object.value(QStringLiteral("lastUpdated")).toString(), Qt::ISODate);

    return asset;

}



QJsonObject sensitivityPointToJson(const SensitivityPoint& point)

{

    QJsonArray holdings;

    for (const auto& holding : point.holdings) {

        holdings.append(holdingToJson(holding));

    }



    return {

        {QStringLiteral("yield"), point.yield},

        {QStringLiteral("beta"), point.beta},

        {QStringLiteral("achievedYield"), point.achievedYield},

        {QStringLiteral("holdings"), holdings}

    };

}



SensitivityPoint sensitivityPointFromJson(const QJsonObject& object)

{

    SensitivityPoint point;

    point.yield = object.value(QStringLiteral("yield")).toDouble();

    point.beta = object.value(QStringLiteral("beta")).toDouble();

    point.achievedYield = object.value(QStringLiteral("achievedYield")).toDouble();



    const auto holdings = object.value(QStringLiteral("holdings")).toArray();

    for (const auto& value : holdings) {

        point.holdings.append(holdingFromJson(value.toObject()));

    }



    return point;

}



QJsonObject sensitivityCurveToJson(const SensitivityCurve& curve)

{

    QJsonArray points;

    for (const auto& point : curve.points) {

        points.append(sensitivityPointToJson(point));

    }



    return {

        {QStringLiteral("minYield"), curve.minYield},

        {QStringLiteral("maxYield"), curve.maxYield},

        {QStringLiteral("step"), curve.step},

        {QStringLiteral("points"), points},

        {QStringLiteral("computedAt"), curve.computedAt.toString(Qt::ISODate)}

    };

}



SensitivityCurve sensitivityCurveFromJson(const QJsonObject& object)

{

    SensitivityCurve curve;

    curve.minYield = object.value(QStringLiteral("minYield")).toDouble();

    curve.maxYield = object.value(QStringLiteral("maxYield")).toDouble();

    curve.step = object.value(QStringLiteral("step")).toDouble();

    curve.computedAt = QDateTime::fromString(object.value(QStringLiteral("computedAt")).toString(), Qt::ISODate);



    const auto points = object.value(QStringLiteral("points")).toArray();

    for (const auto& value : points) {

        curve.points.append(sensitivityPointFromJson(value.toObject()));

    }



    return curve;

}



bool atomicWriteJson(const QString& path, const QJsonDocument& document)

{

    const QString tempPath = path + QStringLiteral(".tmp");

    QFile tempFile(tempPath);

    if (!tempFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {

        return false;

    }



    tempFile.write(document.toJson(QJsonDocument::Indented));

    tempFile.close();



    QFile::remove(path);

    return QFile::rename(tempPath, path);

}



bool atomicWriteBytes(const QString& path, const QByteArray& payload)

{

    const QString tempPath = path + QStringLiteral(".tmp");

    QFile tempFile(tempPath);

    if (!tempFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {

        return false;

    }



    tempFile.write(payload);

    tempFile.close();



    QFile::remove(path);

    return QFile::rename(tempPath, path);

}



QString sanitizeBlobKey(QString key)

{

    key.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9._-]")), QStringLiteral("_"));

    return key;

}

}



bool DataStore::savePortfolio(const Portfolio& portfolio) const

{

    auto portfolios = loadPortfolios();

    const auto it = std::find_if(portfolios.begin(), portfolios.end(), [&](const Portfolio& existing) {

        return existing.id == portfolio.id;

    });



    if (it != portfolios.end()) {

        *it = portfolio;

    } else {

        portfolios.append(portfolio);

    }

    ensureDataDirectory();

    return writePortfolios(portfolioFilePath(), portfolios);

}



QVector<Portfolio> DataStore::loadPortfolios() const

{

    QVector<Portfolio> result;

    QFile file(portfolioFilePath());

    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {

        return result;

    }



    const auto document = QJsonDocument::fromJson(file.readAll());

    for (const auto& value : document.array()) {

        result.append(portfolioFromJson(value.toObject()));

    }



    return result;

}



bool DataStore::deletePortfolio(const QString& id) const

{

    if (id.isEmpty()) {

        return false;

    }



    auto portfolios = loadPortfolios();
    const auto originalSize = portfolios.size();

    portfolios.erase(
        std::remove_if(portfolios.begin(), portfolios.end(), [&](const Portfolio& portfolio) {

            return portfolio.id == id;

        }),
        portfolios.end());

    if (portfolios.size() == originalSize) {

        return false;

    }



    ensureDataDirectory();

    return writePortfolios(portfolioFilePath(), portfolios);

}



bool DataStore::saveAssets(const QVector<Asset>& assets) const

{

    QJsonArray items;

    for (const auto& asset : assets) {

        items.append(assetToJson(asset));

    }



    ensureDataDirectory();

    return atomicWriteJson(assetFilePath(), QJsonDocument(items));

}



QVector<Asset> DataStore::loadAssets() const

{

    QVector<Asset> result;

    QFile file(assetFilePath());

    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {

        return result;

    }



    const auto document = QJsonDocument::fromJson(file.readAll());

    for (const auto& value : document.array()) {

        result.append(assetFromJson(value.toObject()));

    }



    return result;

}



bool DataStore::saveSensitivityCurve(const SensitivityCurve& curve) const

{

    auto curves = loadSensitivityCurves();

    curves.append(curve);



    QJsonArray items;

    for (const auto& item : curves) {

        items.append(sensitivityCurveToJson(item));

    }



    ensureDataDirectory();

    return atomicWriteJson(sensitivityCurveFilePath(), QJsonDocument(items));

}



QVector<SensitivityCurve> DataStore::loadSensitivityCurves() const

{

    QVector<SensitivityCurve> result;

    QFile file(sensitivityCurveFilePath());

    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {

        return result;

    }



    const auto document = QJsonDocument::fromJson(file.readAll());

    for (const auto& value : document.array()) {

        result.append(sensitivityCurveFromJson(value.toObject()));

    }



    return result;

}



bool DataStore::saveBlob(const QString& key, const QByteArray& blob) const

{

    if (key.isEmpty()) {

        return false;

    }



    ensureDataDirectory();

    ensureBlobDirectory();

    return atomicWriteBytes(blobFilePath(key), blob);

}



QByteArray DataStore::loadBlob(const QString& key) const

{

    if (key.isEmpty()) {

        return {};

    }



    QFile file(blobFilePath(key));

    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {

        return {};

    }



    return file.readAll();

}

bool DataStore::saveCachedResponse(const QString& key, const QByteArray& data, int ttlSeconds) const

{

    if (key.isEmpty() || ttlSeconds <= 0 || !saveBlob(key, data)) {

        return false;

    }

    const QJsonObject metadata{
        {QStringLiteral("timestamp"), static_cast<qint64>(QDateTime::currentSecsSinceEpoch())},
        {QStringLiteral("ttl"), ttlSeconds}
    };

    return atomicWriteJson(cacheMetadataFilePath(key), QJsonDocument(metadata));

}

QByteArray DataStore::loadCachedResponse(const QString& key) const

{

    if (!isCacheValid(key)) {
        QFile::remove(blobFilePath(key));
        QFile::remove(cacheMetadataFilePath(key));
        return {};
    }

    return loadBlob(key);

}

bool DataStore::isCacheValid(const QString& key) const

{

    if (key.isEmpty()) {

        return false;

    }

    QFile metadataFile(cacheMetadataFilePath(key));
    if (!metadataFile.exists() || !metadataFile.open(QIODevice::ReadOnly)) {
        return false;
    }

    const QJsonObject metadata = QJsonDocument::fromJson(metadataFile.readAll()).object();
    const qint64 timestamp = metadata.value(QStringLiteral("timestamp")).toVariant().toLongLong();
    const int ttlSeconds = metadata.value(QStringLiteral("ttl")).toInt();
    if (timestamp <= 0 || ttlSeconds <= 0) {
        return false;
    }

    if (!QFile::exists(blobFilePath(key))) {
        return false;
    }

    const qint64 expiresAt = timestamp + ttlSeconds;
    return QDateTime::currentSecsSinceEpoch() <= expiresAt;

}



QString DataStore::dataDirectory() const

{

    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

}



bool DataStore::ensureDataDirectory() const

{

    QDir dir(dataDirectory());

    return dir.exists() || dir.mkpath(QStringLiteral("."));

}



bool DataStore::ensureBlobDirectory() const

{

    QDir dir(dataDirectory());

    return dir.exists(QStringLiteral("blobs")) || dir.mkpath(QStringLiteral("blobs"));

}



QString DataStore::portfolioFilePath() const

{

    return dataDirectory() + QStringLiteral("/portfolios.json");

}



QString DataStore::assetFilePath() const

{

    return dataDirectory() + QStringLiteral("/assets.json");

}



QString DataStore::sensitivityCurveFilePath() const

{

    return dataDirectory() + QStringLiteral("/sensitivity_curves.json");

}



QString DataStore::blobFilePath(const QString& key) const

{

    return dataDirectory() + QStringLiteral("/blobs/") + sanitizeBlobKey(key) + QStringLiteral(".json");

}

QString DataStore::cacheMetadataFilePath(const QString& key) const

{

    return dataDirectory() + QStringLiteral("/blobs/") + sanitizeBlobKey(key) + QStringLiteral(".meta.json");

}
