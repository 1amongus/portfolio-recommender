#include "PortfolioController.h"

#include <QDateTime>
#include <QUuid>

#include "../../utils/Math.h"

PortfolioController::PortfolioController(QObject* parent)
    : QObject(parent)
    , m_targetYield(0.03)
    , m_isLoading(false)
    , m_optimizer(&m_dataStore)
{
    ensureSeedData();
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

void PortfolioController::ensureSeedData()
{
    if (!m_dataStore.loadAssets().isEmpty()) {
        return;
    }

    QVector<Asset> seedAssets = {
        {QStringLiteral("SCHD"), QStringLiteral("Schwab U.S. Dividend Equity ETF"), QStringLiteral("ETF"), 79.0, 0.036, 0.88, 50000000000.0, true, QDateTime::currentDateTimeUtc()},
        {QStringLiteral("VYM"), QStringLiteral("Vanguard High Dividend Yield ETF"), QStringLiteral("ETF"), 118.0, 0.031, 0.85, 54000000000.0, true, QDateTime::currentDateTimeUtc()},
        {QStringLiteral("KO"), QStringLiteral("Coca-Cola"), QStringLiteral("Consumer Defensive"), 63.0, 0.029, 0.58, 270000000000.0, false, QDateTime::currentDateTimeUtc()},
        {QStringLiteral("JNJ"), QStringLiteral("Johnson & Johnson"), QStringLiteral("Healthcare"), 148.0, 0.032, 0.54, 360000000000.0, false, QDateTime::currentDateTimeUtc()},
        {QStringLiteral("PG"), QStringLiteral("Procter & Gamble"), QStringLiteral("Consumer Defensive"), 165.0, 0.025, 0.43, 390000000000.0, false, QDateTime::currentDateTimeUtc()},
        {QStringLiteral("XLU"), QStringLiteral("Utilities Select Sector SPDR Fund"), QStringLiteral("Utilities"), 69.0, 0.034, 0.61, 13000000000.0, true, QDateTime::currentDateTimeUtc()}
    };

    m_dataStore.saveAssets(seedAssets);
}

void PortfolioController::setErrorMessage(const QString& message)
{
    if (m_errorMessage == message) {
        return;
    }

    m_errorMessage = message;
    emit errorMessageChanged();
}