#pragma once

#include <QString>
#include <QVector>

#include "../../models/Asset.h"
#include "../../models/Portfolio.h"

class DataStore
{
public:
    virtual ~DataStore() = default;

    virtual bool savePortfolio(const Portfolio& portfolio) const;
    virtual QVector<Portfolio> loadPortfolios() const;

    virtual bool saveAssets(const QVector<Asset>& assets) const;
    virtual QVector<Asset> loadAssets() const;

    QString dataDirectory() const;

private:
    bool ensureDataDirectory() const;
    QString portfolioFilePath() const;
    QString assetFilePath() const;
};