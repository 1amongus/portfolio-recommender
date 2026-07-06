#include "PortfolioController.h"

#include <QDateTime>
#include <QHash>
#include <QUuid>

#include "../../utils/Math.h"

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
}

PortfolioController::PortfolioController(QObject* parent)
    : QObject(parent)
    , m_targetYield(0.03)
    , m_isLoading(false)
    , m_universeManager(&m_dataStore)
    , m_optimizer(&m_dataStore)
{
    m_universeManager.ensureExpandedSeedData();
}

double PortfolioController::targetYield() const
{
    return m_targetYield;
}

void PortfolioController::setTargetYield(double targetYield)
{
    if (qFuzzyCompare(m_targetYield + 1.0, targetYield + 1.0)) {
        return;
    }

    m_targetYield = targetYield;
    emit targetYieldChanged();
}

QVariantMap PortfolioController::portfolioResult() const
{
    return m_portfolioResult;
}

bool PortfolioController::isLoading() const
{
    return m_isLoading;
}

QString PortfolioController::errorMessage() const
{
    return m_errorMessage;
}

void PortfolioController::generate()
{
    if (m_isLoading) {
        return;
    }

    m_isLoading = true;
    emit isLoadingChanged();
    setErrorMessage(QString());

    const auto holdings = m_optimizer.optimize(m_targetYield, 5);
    if (holdings.isEmpty()) {
        m_portfolioResult.clear();
        emit portfolioResultChanged();
        setErrorMessage(QStringLiteral("No qualifying assets available for the requested yield."));
        m_isLoading = false;
        emit isLoadingChanged();
        emit errorOccurred(m_errorMessage);
        return;
    }

    const QHash<QString, Asset> assetsByTicker = buildAssetLookup(m_dataStore.loadAssets());
    QVariantList holdingsList;
    bool stale = false;
    for (const auto& holding : holdings) {
        const bool holdingStale = isAssetStale(assetsByTicker.value(holding.ticker.toUpper()));
        stale = stale || holdingStale;

        QVariantMap item;
        item.insert(QStringLiteral("ticker"), holding.ticker);
        item.insert(QStringLiteral("weight"), holding.weight);
        item.insert(QStringLiteral("yield"), holding.yield);
        item.insert(QStringLiteral("beta"), holding.beta);
        item.insert(QStringLiteral("stale"), holdingStale);
        holdingsList.append(item);
    }

    Portfolio portfolio;
    portfolio.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    portfolio.name = QStringLiteral("Generated Portfolio");
    portfolio.targetYield = m_targetYield;
    portfolio.holdings = holdings;
    portfolio.aggregateBeta = portfolioBeta(holdings);
    portfolio.achievedYield = portfolioYield(holdings);
    portfolio.createdAt = QDateTime::currentDateTimeUtc();
    m_dataStore.savePortfolio(portfolio);

    m_portfolioResult = {
        {QStringLiteral("holdings"), holdingsList},
        {QStringLiteral("aggregateBeta"), portfolio.aggregateBeta},
        {QStringLiteral("achievedYield"), portfolio.achievedYield},
        {QStringLiteral("targetYield"), portfolio.targetYield},
        {QStringLiteral("name"), portfolio.name},
        {QStringLiteral("stale"), stale}
    };
    emit portfolioResultChanged();

    m_isLoading = false;
    emit isLoadingChanged();
    emit portfolioReady();
}

void PortfolioController::loadPortfolio(const QVariantMap& portfolioData)
{
    const QVariantList holdingsData = portfolioData.value(QStringLiteral("holdings")).toList();
    if (holdingsData.isEmpty()) {
        setErrorMessage(QStringLiteral("The selected portfolio does not contain any holdings."));
        emit errorOccurred(m_errorMessage);
        return;
    }

    const QHash<QString, Asset> assetsByTicker = buildAssetLookup(m_dataStore.loadAssets());
    QVariantList holdingsList;
    holdingsList.reserve(holdingsData.size());

    QVector<Holding> holdings;
    holdings.reserve(holdingsData.size());
    bool stale = false;
    for (const auto& entry : holdingsData) {
        const QVariantMap map = entry.toMap();
        Holding holding;
        holding.ticker = map.value(QStringLiteral("ticker")).toString();
        holding.weight = map.value(QStringLiteral("weight")).toDouble();
        holding.yield = map.value(QStringLiteral("yield")).toDouble();
        holding.beta = map.value(QStringLiteral("beta")).toDouble();
        if (holding.ticker.isEmpty()) {
            continue;
        }

        const bool holdingStale = isAssetStale(assetsByTicker.value(holding.ticker.toUpper()));
        stale = stale || holdingStale;

        holdings.append(holding);
        holdingsList.append(QVariantMap{
            {QStringLiteral("ticker"), holding.ticker},
            {QStringLiteral("weight"), holding.weight},
            {QStringLiteral("yield"), holding.yield},
            {QStringLiteral("beta"), holding.beta},
            {QStringLiteral("stale"), holdingStale}
        });
    }

    if (holdings.isEmpty()) {
        setErrorMessage(QStringLiteral("The selected portfolio does not contain any valid holdings."));
        emit errorOccurred(m_errorMessage);
        return;
    }

    const double targetYield = portfolioData.contains(QStringLiteral("targetYield"))
        ? portfolioData.value(QStringLiteral("targetYield")).toDouble()
        : portfolioData.value(QStringLiteral("achievedYield")).toDouble();
    setTargetYield(targetYield);

    m_portfolioResult = {
        {QStringLiteral("holdings"), holdingsList},
        {QStringLiteral("aggregateBeta"), portfolioData.contains(QStringLiteral("aggregateBeta")) ? portfolioData.value(QStringLiteral("aggregateBeta")).toDouble() : portfolioBeta(holdings)},
        {QStringLiteral("achievedYield"), portfolioData.contains(QStringLiteral("achievedYield")) ? portfolioData.value(QStringLiteral("achievedYield")).toDouble() : portfolioYield(holdings)},
        {QStringLiteral("targetYield"), targetYield},
        {QStringLiteral("name"), portfolioData.value(QStringLiteral("name")).toString().isEmpty() ? QStringLiteral("Loaded Portfolio") : portfolioData.value(QStringLiteral("name")).toString()},
        {QStringLiteral("stale"), stale}
    };
    emit portfolioResultChanged();

    setErrorMessage(QString());
    emit portfolioReady();
}

void PortfolioController::setErrorMessage(const QString& message)
{
    if (m_errorMessage == message) {
        return;
    }

    m_errorMessage = message;
    emit errorMessageChanged();
}
