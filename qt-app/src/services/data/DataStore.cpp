#include "DataStore.h"

#include <algorithm>

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

namespace {
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

    QJsonArray items;
    for (const auto& item : portfolios) {
        items.append(portfolioToJson(item));
    }

    ensureDataDirectory();
    return atomicWriteJson(portfolioFilePath(), QJsonDocument(items));
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

QString DataStore::dataDirectory() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

bool DataStore::ensureDataDirectory() const
{
    QDir dir(dataDirectory());
    return dir.exists() || dir.mkpath(QStringLiteral("."));
}

QString DataStore::portfolioFilePath() const
{
    return dataDirectory() + QStringLiteral("/portfolios.json");
}

QString DataStore::assetFilePath() const
{
    return dataDirectory() + QStringLiteral("/assets.json");
}