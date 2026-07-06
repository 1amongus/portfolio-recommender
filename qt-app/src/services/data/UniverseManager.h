#pragma once

#include <QObject>
#include <QFuture>
#include <QVector>

#include "../../models/Asset.h"
#include "DataStore.h"
#include "providers/AlphaVantageProvider.h"
#include "providers/FmpProvider.h"
#include "providers/PolygonProvider.h"

class UniverseManager : public QObject
{
    Q_OBJECT

public:
    explicit UniverseManager(DataStore* dataStore, QObject* parent = nullptr);

    QVector<Asset> assets() const;
    int assetCount() const;
    bool hasApiKey() const;

    QFuture<bool> refreshTicker(const QString& ticker);
    QFuture<int> refreshAll();
    QFuture<bool> addTicker(const QString& ticker);
    bool removeTicker(const QString& ticker);

    void ensureExpandedSeedData();

signals:
    void refreshProgress(int current, int total);
    void refreshComplete(int updated);
    void errorOccurred(const QString& message);

private:
    Asset fetchAssetFromProviders(const QString& ticker);
    DataStore* m_dataStore;
    AlphaVantageProvider m_alphaVantageProvider;
    FmpProvider m_fmpProvider;
    PolygonProvider m_polygonProvider;
};
