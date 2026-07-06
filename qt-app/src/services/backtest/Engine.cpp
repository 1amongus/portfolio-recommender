#include "Engine.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include <QDateTime>
#include <QHash>
#include <QMap>
#include <QRandomGenerator>
#include <QStringList>
#include <QtMath>

#include "../../models/Asset.h"
#include "../../services/data/DataStore.h"
#include "../../utils/Math.h"

namespace {
constexpr double kAnnualMarketVolatility = 0.15;
constexpr double kTradingDaysPerYear = 252.0;
constexpr double kAnnualRiskFreeRate = 0.02;
constexpr double kPi = 3.14159265358979323846;

QVector<QDate> tradingDatesBetween(const QDate& startDate, const QDate& endDate)
{
    QVector<QDate> dates;
    if (!startDate.isValid() || !endDate.isValid() || startDate > endDate) {
        return dates;
    }

    for (QDate date = startDate; date <= endDate; date = date.addDays(1)) {
        const int dayOfWeek = date.dayOfWeek();
        if (dayOfWeek >= 1 && dayOfWeek <= 5) {
            dates.append(date);
        }
    }

    return dates;
}

double mean(const QVector<double>& values)
{
    if (values.isEmpty()) {
        return 0.0;
    }

    double total = 0.0;
    for (double value : values) {
        total += value;
    }
    return total / static_cast<double>(values.size());
}

double sampleStandardDeviation(const QVector<double>& values)
{
    if (values.size() < 2) {
        return 0.0;
    }

    const double avg = mean(values);
    double sumSquares = 0.0;
    for (double value : values) {
        const double delta = value - avg;
        sumSquares += delta * delta;
    }

    return qSqrt(sumSquares / static_cast<double>(values.size() - 1));
}

double nextNormal(QRandomGenerator& generator)
{
    const double u1 = qMax(generator.generateDouble(), std::numeric_limits<double>::min());
    const double u2 = generator.generateDouble();
    return qSqrt(-2.0 * qLn(u1)) * qCos(2.0 * kPi * u2);
}

QString portfolioIdentifier(const QVector<Holding>& holdings)
{
    QStringList parts;
    parts.reserve(holdings.size());
    for (const auto& holding : holdings) {
        parts.append(QStringLiteral("%1:%2").arg(holding.ticker, QString::number(holding.weight, 'f', 4)));
    }
    return parts.join(QStringLiteral("|"));
}

bool isQuarterStart(const QDate& date)
{
    return date.month() == 1 || date.month() == 4 || date.month() == 7 || date.month() == 10;
}

bool shouldRebalanceOnDate(const QDate& currentDate, const QDate& previousDate, RebalanceFrequency frequency)
{
    if (!currentDate.isValid() || !previousDate.isValid() || frequency == RebalanceFrequency::Never) {
        return false;
    }

    switch (frequency) {
    case RebalanceFrequency::Monthly:
        return currentDate.month() != previousDate.month() || currentDate.year() != previousDate.year();
    case RebalanceFrequency::Quarterly:
        return currentDate.year() != previousDate.year()
            || (currentDate.month() != previousDate.month() && isQuarterStart(currentDate));
    case RebalanceFrequency::Annually:
        return currentDate.year() != previousDate.year();
    case RebalanceFrequency::Never:
    default:
        return false;
    }
}

QString rebalanceFrequencyKey(RebalanceFrequency frequency)
{
    switch (frequency) {
    case RebalanceFrequency::Monthly:
        return QStringLiteral("monthly");
    case RebalanceFrequency::Quarterly:
        return QStringLiteral("quarterly");
    case RebalanceFrequency::Annually:
        return QStringLiteral("annually");
    case RebalanceFrequency::Never:
    default:
        return QStringLiteral("never");
    }
}
}

BacktestEngine::BacktestEngine(DataStore* dataStore)
    : m_dataStore(dataStore)
{
}

BacktestResult BacktestEngine::run(const QVector<Holding>& holdings,
                                   const QDate& startDate,
                                   const QDate& endDate,
                                   RebalanceFrequency rebalance) const
{
    BacktestResult result;
    result.startDate = startDate;
    result.endDate = endDate;
    result.computedAt = QDateTime::currentDateTimeUtc();

    if (holdings.isEmpty() || !startDate.isValid() || !endDate.isValid() || startDate > endDate) {
        return result;
    }

    const QVector<QDate> tradingDates = tradingDatesBetween(startDate, endDate);
    if (tradingDates.isEmpty()) {
        return result;
    }

    double totalWeight = 0.0;
    for (const auto& holding : holdings) {
        totalWeight += holding.weight;
    }
    if (totalWeight <= 0.0) {
        return result;
    }

    const QVector<Asset> assets = m_dataStore != nullptr ? m_dataStore->loadAssets() : QVector<Asset>{};
    QMap<QString, Asset> assetByTicker;
    for (const auto& asset : assets) {
        assetByTicker.insert(asset.ticker.toUpper(), asset);
    }

    QVector<Holding> normalizedHoldings;
    normalizedHoldings.reserve(holdings.size());
    QVector<QVector<double>> priceSeries;
    priceSeries.reserve(holdings.size());
    QVector<double> basePrices;
    basePrices.reserve(holdings.size());
    QVector<double> shares;
    shares.reserve(holdings.size());

    for (const auto& originalHolding : holdings) {
        const Asset asset = assetByTicker.value(originalHolding.ticker.toUpper());

        Holding holding = originalHolding;
        holding.ticker = holding.ticker.toUpper();
        holding.weight /= totalWeight;
        if (qFuzzyIsNull(holding.yield) && asset.dividendYield > 0.0) {
            holding.yield = asset.dividendYield;
        }
        if (qFuzzyIsNull(holding.beta) && asset.beta > 0.0) {
            holding.beta = asset.beta;
        }

        normalizedHoldings.append(holding);
        priceSeries.append(generateSyntheticPrices(holding.ticker, holding.beta, holding.yield, tradingDates.size()));
        basePrices.append(asset.price > 0.0 ? asset.price : 100.0);
        shares.append(qFuzzyIsNull(priceSeries.last().value(0)) ? 0.0 : holding.weight / priceSeries.last().first());
    }

    QVector<double> marketCurve = generateSyntheticPrices(QStringLiteral("MARKET"), 1.0, 0.02, tradingDates.size());
    QVector<double> effectiveYields;
    effectiveYields.reserve(tradingDates.size());

    double peak = 0.0;
    for (int day = 0; day < tradingDates.size(); ++day) {
        double portfolioValue = 0.0;

        for (int index = 0; index < normalizedHoldings.size(); ++index) {
            const auto& prices = priceSeries[index];
            if (prices.isEmpty()) {
                continue;
            }

            portfolioValue += shares[index] * prices[day];
        }

        if (day > 0 && shouldRebalanceOnDate(tradingDates[day], tradingDates[day - 1], rebalance) && portfolioValue > 0.0) {
            for (int index = 0; index < normalizedHoldings.size(); ++index) {
                const auto& prices = priceSeries[index];
                if (prices.isEmpty() || qFuzzyIsNull(prices[day])) {
                    continue;
                }

                shares[index] = (portfolioValue * normalizedHoldings[index].weight) / prices[day];
            }
        }

        portfolioValue = 0.0;
        double effectiveYield = 0.0;
        for (int index = 0; index < normalizedHoldings.size(); ++index) {
            const auto& holding = normalizedHoldings[index];
            const auto& prices = priceSeries[index];
            if (prices.isEmpty() || qFuzzyIsNull(prices[day])) {
                continue;
            }

            const double positionValue = shares[index] * prices[day];
            portfolioValue += positionValue;
        }

        if (portfolioValue > 0.0) {
            for (int index = 0; index < normalizedHoldings.size(); ++index) {
                const auto& holding = normalizedHoldings[index];
                const auto& prices = priceSeries[index];
                if (prices.isEmpty() || qFuzzyIsNull(prices[day])) {
                    continue;
                }

                const double positionValue = shares[index] * prices[day];
                const double positionWeight = positionValue / portfolioValue;
                const double annualDividendPerUnit = basePrices[index] * holding.yield;
                effectiveYield += positionWeight * (annualDividendPerUnit / prices[day]);
            }
        }

        result.equityCurve.append(portfolioValue);
        peak = qMax(peak, portfolioValue);
        result.drawdownCurve.append(qFuzzyIsNull(peak) ? 0.0 : (portfolioValue - peak) / peak);
        effectiveYields.append(effectiveYield);

        const int windowStart = qMax(0, day - static_cast<int>(kTradingDaysPerYear) + 1);
        QVector<double> window;
        window.reserve(day - windowStart + 1);
        for (int offset = windowStart; offset <= day; ++offset) {
            window.append(effectiveYields[offset]);
        }
        result.rollingYield.append(mean(window));
    }

    result.dates = tradingDates;
    result.portfolioId = QStringLiteral("%1_%2").arg(portfolioIdentifier(normalizedHoldings), rebalanceFrequencyKey(rebalance));

    QVector<double> returns;
    returns.reserve(result.equityCurve.size() > 1 ? result.equityCurve.size() - 1 : 0);
    for (qsizetype i = 1; i < result.equityCurve.size(); ++i) {
        if (qFuzzyIsNull(result.equityCurve[i - 1])) {
            continue;
        }
        returns.append((result.equityCurve[i] - result.equityCurve[i - 1]) / result.equityCurve[i - 1]);
    }

    if (!result.equityCurve.isEmpty() && result.equityCurve.first() > 0.0 && result.equityCurve.last() > 0.0) {
        result.metrics.totalReturn = (result.equityCurve.last() / result.equityCurve.first()) - 1.0;
        if (!returns.isEmpty()) {
            result.metrics.annualizedReturn =
                qPow(result.equityCurve.last() / result.equityCurve.first(),
                     kTradingDaysPerYear / static_cast<double>(returns.size())) - 1.0;
        }
    }

    const double dailyRiskFreeRate = kAnnualRiskFreeRate / kTradingDaysPerYear;
    result.metrics.maxDrawdown = calculateMaxDrawdown(result.equityCurve);
    result.metrics.sharpeRatio = calculateSharpe(returns, dailyRiskFreeRate) * qSqrt(kTradingDaysPerYear);
    result.metrics.sortinoRatio = calculateSortino(returns, dailyRiskFreeRate) * qSqrt(kTradingDaysPerYear);
    result.metrics.averageYield = mean(result.rollingYield);
    result.metrics.yieldStability = sampleStandardDeviation(result.rollingYield);

    const double realizedBeta = calculateBeta(result.equityCurve, marketCurve);
    result.metrics.portfolioBeta = qFuzzyIsNull(realizedBeta) ? portfolioBeta(normalizedHoldings) : realizedBeta;

    return result;
}

QVector<double> BacktestEngine::generateSyntheticPrices(double beta, double yield, int tradingDays) const
{
    return generateSyntheticPrices(QStringLiteral("DEFAULT"), beta, yield, tradingDays);
}

QVector<double> BacktestEngine::generateSyntheticPrices(const QString& ticker, double beta, double yield, int tradingDays) const
{
    QVector<double> prices;
    if (tradingDays <= 0) {
        return prices;
    }

    prices.reserve(tradingDays);
    prices.append(100.0);

    const quint32 seed = qHash(ticker.toUpper());
    QRandomGenerator generator(seed);
    const double dailyVolatility = (kAnnualMarketVolatility * qMax(0.25, beta > 0.0 ? beta : 1.0)) / qSqrt(kTradingDaysPerYear);
    const double dailyDrift = yield / kTradingDaysPerYear;

    for (int day = 1; day < tradingDays; ++day) {
        const double shock = nextNormal(generator) * dailyVolatility;
        const double dailyReturn = qMax(-0.95, dailyDrift + shock);
        prices.append(prices.last() * (1.0 + dailyReturn));
    }

    return prices;
}
