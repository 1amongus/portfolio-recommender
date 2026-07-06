#pragma once

#include <QString>

#include "../../models/BacktestResult.h"
#include "../../models/Portfolio.h"

class PdfExporter
{
public:
    static bool exportPortfolioReport(const Portfolio& portfolio, const BacktestMetrics* metrics, const QString& filePath);
};
