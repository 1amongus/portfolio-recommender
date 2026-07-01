#include "PortfolioController.h"

#include <QDateTime>
#include <QUuid>

#include "../../utils/Math.h"

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

    QVariantList holdingsList;
    for (const auto& holding : holdings) {
        QVariantMap item;
        item.insert(QStringLiteral("ticker"), holding.ticker);
        item.insert(QStringLiteral("weight"), holding.weight);
        item.insert(QStringLiteral("yield"), holding.yield);
        item.insert(QStringLiteral("beta"), holding.beta);
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
        {QStringLiteral("name"), portfolio.name}
    };
    emit portfolioResultChanged();

    m_isLoading = false;
    emit isLoadingChanged();
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