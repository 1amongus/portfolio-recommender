#include "SavedController.h"

#include <QDateTime>
#include <QHash>

namespace {
bool isAssetStale(const Asset& asset)
{
    return !asset.lastUpdated.isValid() || asset.lastUpdated.addDays(30) < QDateTime::currentDateTimeUtc();
}

QHash<QString, Asset> buildAssetLookup(const QVector<Asset>& assets)
{
    QHash<QString, Asset> lookup;
    lookup.reserve(assets.size());
    for (const auto& asset : assets) {
        lookup.insert(asset.ticker.toUpper(), asset);
    }

    return lookup;
}

QVariantList holdingsToVariantList(const QVector<Holding>& holdings, const QHash<QString, Asset>& assetsByTicker, bool* hasStaleAsset)
{
    QVariantList list;
    list.reserve(holdings.size());

    for (const auto& holding : holdings) {
        const bool stale = isAssetStale(assetsByTicker.value(holding.ticker.toUpper()));
        if (hasStaleAsset != nullptr) {
            *hasStaleAsset = *hasStaleAsset || stale;
        }

        list.append(QVariantMap{
            {QStringLiteral("ticker"), holding.ticker},
            {QStringLiteral("weight"), holding.weight},
            {QStringLiteral("yield"), holding.yield},
            {QStringLiteral("beta"), holding.beta},
            {QStringLiteral("stale"), stale}
        });
    }

    return list;
}

QVariantMap portfolioSummaryToVariant(const Portfolio& portfolio, const QHash<QString, Asset>& assetsByTicker)
{
    bool stale = false;
    for (const auto& holding : portfolio.holdings) {
        stale = stale || isAssetStale(assetsByTicker.value(holding.ticker.toUpper()));
    }

    return {
        {QStringLiteral("id"), portfolio.id},
        {QStringLiteral("name"), portfolio.name},
        {QStringLiteral("targetYield"), portfolio.targetYield},
        {QStringLiteral("achievedYield"), portfolio.achievedYield},
        {QStringLiteral("aggregateBeta"), portfolio.aggregateBeta},
        {QStringLiteral("holdingsCount"), portfolio.holdings.size()},
        {QStringLiteral("createdAt"), portfolio.createdAt.toLocalTime().toString(QStringLiteral("yyyy-MM-dd hh:mm"))},
        {QStringLiteral("stale"), stale}
    };
}

QVariantMap portfolioDetailToVariant(const Portfolio& portfolio, const QHash<QString, Asset>& assetsByTicker)
{
    bool stale = false;
    return {
        {QStringLiteral("id"), portfolio.id},
        {QStringLiteral("name"), portfolio.name},
        {QStringLiteral("targetYield"), portfolio.targetYield},
        {QStringLiteral("achievedYield"), portfolio.achievedYield},
        {QStringLiteral("aggregateBeta"), portfolio.aggregateBeta},
        {QStringLiteral("holdingsCount"), portfolio.holdings.size()},
        {QStringLiteral("holdings"), holdingsToVariantList(portfolio.holdings, assetsByTicker, &stale)},
        {QStringLiteral("createdAt"), portfolio.createdAt.toLocalTime().toString(QStringLiteral("yyyy-MM-dd hh:mm"))},
        {QStringLiteral("stale"), stale}
    };
}
}

SavedController::SavedController(QObject* parent)
    : QObject(parent)
{
    refresh();
}

QVariantList SavedController::portfolios() const
{
    return m_portfolios;
}

QVariantMap SavedController::selectedPortfolio() const
{
    return m_selectedPortfolio;
}

int SavedController::count() const
{
    return m_portfolios.size();
}

void SavedController::refresh()
{
    const QVector<Portfolio> portfolios = m_dataStore.loadPortfolios();
    const QHash<QString, Asset> assetsByTicker = buildAssetLookup(m_dataStore.loadAssets());
    QVariantList refreshed;
    refreshed.reserve(portfolios.size());

    QString selectedId = m_selectedPortfolio.value(QStringLiteral("id")).toString();
    int selectedIndex = -1;

    for (int index = 0; index < portfolios.size(); ++index) {
        const Portfolio& portfolio = portfolios.at(index);
        refreshed.append(portfolioSummaryToVariant(portfolio, assetsByTicker));
        if (!selectedId.isEmpty() && portfolio.id == selectedId) {
            selectedIndex = index;
        }
    }

    m_portfolios = refreshed;
    m_selectedIndex = selectedIndex;
    emit portfoliosChanged();

    if (m_selectedIndex >= 0) {
        selectPortfolio(m_selectedIndex);
        return;
    }

    if (!m_selectedPortfolio.isEmpty()) {
        m_selectedPortfolio.clear();
        emit selectedPortfolioChanged();
    }
}

void SavedController::selectPortfolio(int index)
{
    const QVector<Portfolio> portfolios = m_dataStore.loadPortfolios();
    const QHash<QString, Asset> assetsByTicker = buildAssetLookup(m_dataStore.loadAssets());
    if (index < 0 || index >= portfolios.size()) {
        if (m_selectedIndex != -1 || !m_selectedPortfolio.isEmpty()) {
            m_selectedIndex = -1;
            m_selectedPortfolio.clear();
            emit selectedPortfolioChanged();
        }
        return;
    }

    m_selectedIndex = index;
    m_selectedPortfolio = portfolioDetailToVariant(portfolios.at(index), assetsByTicker);
    emit selectedPortfolioChanged();
}

void SavedController::deletePortfolio(int index)
{
    const QVector<Portfolio> portfolios = m_dataStore.loadPortfolios();
    if (index < 0 || index >= portfolios.size()) {
        return;
    }

    m_dataStore.deletePortfolio(portfolios.at(index).id);
    if (m_selectedIndex == index) {
        m_selectedIndex = -1;
        if (!m_selectedPortfolio.isEmpty()) {
            m_selectedPortfolio.clear();
            emit selectedPortfolioChanged();
        }
    }

    refresh();
}

void SavedController::renamePortfolio(int index, const QString& newName)
{
    const QString trimmedName = newName.trimmed();
    if (trimmedName.isEmpty()) {
        return;
    }

    QVector<Portfolio> portfolios = m_dataStore.loadPortfolios();
    if (index < 0 || index >= portfolios.size()) {
        return;
    }

    Portfolio portfolio = portfolios.at(index);
    if (portfolio.name == trimmedName) {
        return;
    }

    portfolio.name = trimmedName;
    m_dataStore.savePortfolio(portfolio);
    refresh();
}
