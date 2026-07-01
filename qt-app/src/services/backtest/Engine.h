#pragma once

#include <QDate>
#include <QString>
#include <QVector>

#include "../../models/BacktestResult.h"
#include "../../models/Portfolio.h"

class DataStore;

class BacktestEngine
{
public:
    explicit BacktestEngine(DataStore* dataStore);

    BacktestResult run(const QVector<Holding>& holdings, const QDate& startDate, const QDate& endDate) const;

private:
    QVector<double> generateSyntheticPrices(double beta, double yield, int tradingDays) const;
    QVector<double> generateSyntheticPrices(const QString& ticker, double beta, double yield, int tradingDays) const;

    DataStore* m_dataStore;
};
