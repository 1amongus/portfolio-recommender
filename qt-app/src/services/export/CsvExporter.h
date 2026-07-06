#pragma once

#include <QString>
#include <QVector>

#include "../../models/BacktestResult.h"
#include "../../models/Portfolio.h"

class CsvExporter
{
public:
    static bool exportPortfolio(const Portfolio& portfolio, const QString& filePath);
    static bool exportBacktest(const BacktestResult& result, const QString& filePath);
    static bool exportHoldings(const QVector<Holding>& holdings, const QString& filePath);
};
