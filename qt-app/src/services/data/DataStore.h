#pragma once



#include <QByteArray>

#include <QString>

#include <QVector>



#include "../../models/Asset.h"

#include "../../models/Portfolio.h"

#include "../../models/SensitivityPoint.h"



class DataStore

{

public:

    virtual ~DataStore() = default;



    virtual bool savePortfolio(const Portfolio& portfolio) const;

    virtual QVector<Portfolio> loadPortfolios() const;

    virtual bool deletePortfolio(const QString& id) const;



    virtual bool saveAssets(const QVector<Asset>& assets) const;

    virtual QVector<Asset> loadAssets() const;



    virtual bool saveSensitivityCurve(const SensitivityCurve& curve) const;

    virtual QVector<SensitivityCurve> loadSensitivityCurves() const;



    virtual bool saveBlob(const QString& key, const QByteArray& blob) const;

    virtual QByteArray loadBlob(const QString& key) const;



    QString dataDirectory() const;



private:

    bool ensureDataDirectory() const;

    bool ensureBlobDirectory() const;

    QString portfolioFilePath() const;

    QString assetFilePath() const;

    QString sensitivityCurveFilePath() const;

    QString blobFilePath(const QString& key) const;

};
