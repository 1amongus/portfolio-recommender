#include "SavedController.h"

#include <QDateTime>

namespace {
QVariantList holdingsToVariantList(const QVector<Holding>& holdings)
{
    QVariantList list;
    list.reserve(holdings.size());

    for (const auto& holding : holdings) {
        list.append(QVariantMap{
            {QStringLiteral("ticker"), holding.ticker},
            {QStringLiteral("weight"), holding.weight},
            {QStringLiteral("yield"), holding.yield},
            {QStringLiteral("beta"), holding.beta}
        });
    }

    return list;
}

QVariantMap portfolioSummaryToVariant(const Portfolio& portfolio)
{
    return {
        {QStringLiteral("id"), portfolio.id},
        {QStringLiteral("name"), portfolio.name},
        {QStringLiteral("targetYield"), portfolio.targetYield},
        {QStringLiteral("achievedYield"), portfolio.achievedYield},
        {QStringLiteral("aggregateBeta"), portfolio.aggregateBeta},
        {QStringLiteral("holdingsCount"), portfolio.holdings.size()},
        {QStringLiteral("createdAt"), portfolio.createdAt.toLocalTime().toString(QStringLiteral("yyyy-MM-dd hh:mm"))}
    };
}

QVariantMap portfolioDetailToVariant(const Portfolio& portfolio)
{
    return {
        {QStringLiteral("id"), portfolio.id},
        {QStringLiteral("name"), portfolio.name},
        {QStringLiteral("targetYield"), portfolio.targetYield},
        {QStringLiteral("achievedYield"), portfolio.achievedYield},
        {QStringLiteral("aggregateBeta"), portfolio.aggregateBeta},
        {QStringLiteral("holdingsCount"), portfolio.holdings.size()},
        {QStringLiteral("holdings"), holdingsToVariantList(portfolio.holdings)},
        {QStringLiteral("createdAt"), portfolio.createdAt.toLocalTime().toString(QStringLiteral("yyyy-MM-dd hh:mm"))}
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
    QVariantList refreshed;
    refreshed.reserve(portfolios.size());

    QString selectedId = m_selectedPortfolio.value(QStringLiteral("id")).toString();
    int selectedIndex = -1;

    for (int index = 0; index < portfolios.size(); ++index) {
        const Portfolio& portfolio = portfolios.at(index);
        refreshed.append(portfolioSummaryToVariant(portfolio));
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
    if (index < 0 || index >= portfolios.size()) {
        if (m_selectedIndex != -1 || !m_selectedPortfolio.isEmpty()) {
            m_selectedIndex = -1;
            m_selectedPortfolio.clear();
            emit selectedPortfolioChanged();
        }
        return;
    }

    m_selectedIndex = index;
    m_selectedPortfolio = portfolioDetailToVariant(portfolios.at(index));
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
