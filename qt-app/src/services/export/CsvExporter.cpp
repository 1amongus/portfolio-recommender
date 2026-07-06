#include "CsvExporter.h"

#include <algorithm>

#include <QFile>
#include <QStringConverter>
#include <QStringList>
#include <QTextStream>

namespace {
QString quoteCsv(QString value)
{
    value.replace(QStringLiteral("\""), QStringLiteral("\"\""));
    return QStringLiteral("\"%1\"").arg(value);
}

QString numberString(double value)
{
    return QString::number(value, 'f', 6);
}

bool writeRow(QTextStream& stream, const QStringList& columns)
{
    QStringList escaped;
    escaped.reserve(columns.size());
    for (const auto& column : columns) {
        escaped.append(quoteCsv(column));
    }

    stream << escaped.join(QLatin1Char(',')) << Qt::endl;
    return stream.status() == QTextStream::Ok;
}
}

bool CsvExporter::exportPortfolio(const Portfolio& portfolio, const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    if (!writeRow(stream, {QStringLiteral("Ticker"), QStringLiteral("Weight"), QStringLiteral("Yield"), QStringLiteral("Beta")})) {
        return false;
    }

    double totalWeight = 0.0;
    for (const auto& holding : portfolio.holdings) {
        totalWeight += holding.weight;
        if (!writeRow(stream, {
                holding.ticker,
                numberString(holding.weight),
                numberString(holding.yield),
                numberString(holding.beta) })) {
            return false;
        }
    }

    if (!writeRow(stream, {
            QStringLiteral("Summary"),
            numberString(totalWeight),
            numberString(portfolio.achievedYield),
            numberString(portfolio.aggregateBeta) })) {
        return false;
    }

    file.close();
    return file.error() == QFileDevice::NoError;
}

bool CsvExporter::exportBacktest(const BacktestResult& result, const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    if (!writeRow(stream, {QStringLiteral("Date"), QStringLiteral("PortfolioValue"), QStringLiteral("Drawdown"), QStringLiteral("RollingYield")})) {
        return false;
    }

    const int rowCount = std::min({result.dates.size(), result.equityCurve.size(), result.drawdownCurve.size(), result.rollingYield.size()});
    for (int index = 0; index < rowCount; ++index) {
        if (!writeRow(stream, {
                result.dates.at(index).toString(Qt::ISODate),
                numberString(result.equityCurve.at(index)),
                numberString(result.drawdownCurve.at(index)),
                numberString(result.rollingYield.at(index)) })) {
            return false;
        }
    }

    file.close();
    return file.error() == QFileDevice::NoError;
}

bool CsvExporter::exportHoldings(const QVector<Holding>& holdings, const QString& filePath)
{
    Portfolio portfolio;
    portfolio.holdings = holdings;
    for (const auto& holding : holdings) {
        portfolio.achievedYield += holding.weight * holding.yield;
        portfolio.aggregateBeta += holding.weight * holding.beta;
    }

    return exportPortfolio(portfolio, filePath);
}
