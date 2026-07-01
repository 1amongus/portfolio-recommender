#include "Math.h"

#include <QtMath>

namespace {
QVector<double> toReturns(const QVector<double>& prices)
{
    QVector<double> returns;
    for (int i = 1; i < prices.size(); ++i) {
        if (qFuzzyIsNull(prices[i - 1])) {
            continue;
        }
        returns.append((prices[i] - prices[i - 1]) / prices[i - 1]);
    }
    return returns;
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

double sampleVariance(const QVector<double>& values, double average)
{
    if (values.size() < 2) {
        return 0.0;
    }

    double variance = 0.0;
    for (double value : values) {
        const double delta = value - average;
        variance += delta * delta;
    }
    return variance / static_cast<double>(values.size() - 1);
}
}

namespace Math {
double calculateBeta(const QVector<double>& prices, const QVector<double>& marketPrices)
{
    const auto assetReturns = toReturns(prices);
    const auto benchmarkReturns = toReturns(marketPrices);
    const int count = qMin(assetReturns.size(), benchmarkReturns.size());
    if (count < 2) {
        return 0.0;
    }

    QVector<double> trimmedAssetReturns;
    QVector<double> trimmedBenchmarkReturns;
    trimmedAssetReturns.reserve(count);
    trimmedBenchmarkReturns.reserve(count);
    for (int i = 0; i < count; ++i) {
        trimmedAssetReturns.append(assetReturns[i]);
        trimmedBenchmarkReturns.append(benchmarkReturns[i]);
    }

    const double assetMean = mean(trimmedAssetReturns);
    const double benchmarkMean = mean(trimmedBenchmarkReturns);

    double covariance = 0.0;
    for (int i = 0; i < count; ++i) {
        covariance += (trimmedAssetReturns[i] - assetMean) * (trimmedBenchmarkReturns[i] - benchmarkMean);
    }
    covariance /= static_cast<double>(count - 1);

    const double benchmarkVariance = sampleVariance(trimmedBenchmarkReturns, benchmarkMean);
    if (qFuzzyIsNull(benchmarkVariance)) {
        return 0.0;
    }

    return covariance / benchmarkVariance;
}

double calculateSharpe(const QVector<double>& returns, double riskFreeRate)
{
    if (returns.isEmpty()) {
        return 0.0;
    }

    const double avg = mean(returns);
    const double variance = sampleVariance(returns, avg);
    const double stddev = qSqrt(variance);
    if (qFuzzyIsNull(stddev)) {
        return 0.0;
    }
    return (avg - riskFreeRate) / stddev;
}

double calculateSortino(const QVector<double>& returns, double riskFreeRate)
{
    if (returns.isEmpty()) {
        return 0.0;
    }

    QVector<double> downside;
    downside.reserve(returns.size());
    for (double value : returns) {
        if (value < riskFreeRate) {
            downside.append(value - riskFreeRate);
        }
    }

    if (downside.isEmpty()) {
        return 0.0;
    }

    double squares = 0.0;
    for (double value : downside) {
        squares += value * value;
    }
    const double downsideDeviation = qSqrt(squares / static_cast<double>(downside.size()));
    if (qFuzzyIsNull(downsideDeviation)) {
        return 0.0;
    }

    return (mean(returns) - riskFreeRate) / downsideDeviation;
}

double calculateMaxDrawdown(const QVector<double>& prices)
{
    if (prices.isEmpty()) {
        return 0.0;
    }

    double peak = prices.first();
    double maxDrawdown = 0.0;
    for (double price : prices) {
        peak = qMax(peak, price);
        if (!qFuzzyIsNull(peak)) {
            maxDrawdown = qMin(maxDrawdown, (price - peak) / peak);
        }
    }
    return maxDrawdown;
}

double portfolioBeta(const QVector<Holding>& holdings)
{
    double result = 0.0;
    for (const auto& holding : holdings) {
        result += holding.weight * holding.beta;
    }
    return result;
}

double portfolioYield(const QVector<Holding>& holdings)
{
    double result = 0.0;
    for (const auto& holding : holdings) {
        result += holding.weight * holding.yield;
    }
    return result;
}
}

double calculateBeta(const QVector<double>& prices, const QVector<double>& marketPrices)
{
    return Math::calculateBeta(prices, marketPrices);
}

double calculateSharpe(const QVector<double>& returns, double riskFreeRate)
{
    return Math::calculateSharpe(returns, riskFreeRate);
}

double calculateSortino(const QVector<double>& returns, double riskFreeRate)
{
    return Math::calculateSortino(returns, riskFreeRate);
}

double calculateMaxDrawdown(const QVector<double>& prices)
{
    return Math::calculateMaxDrawdown(prices);
}

double portfolioBeta(const QVector<Holding>& holdings)
{
    return Math::portfolioBeta(holdings);
}

double portfolioYield(const QVector<Holding>& holdings)
{
    return Math::portfolioYield(holdings);
}