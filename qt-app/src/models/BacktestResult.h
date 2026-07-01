#pragma once

#include <QDate>
#include <QDateTime>
#include <QString>
#include <QVector>

struct BacktestMetrics {
    double totalReturn = 0.0;
    double annualizedReturn = 0.0;
    double maxDrawdown = 0.0;
    double sharpeRatio = 0.0;
    double sortinoRatio = 0.0;
    double averageYield = 0.0;
    double yieldStability = 0.0;
    double portfolioBeta = 0.0;
};

struct BacktestResult {
    QString portfolioId;
    QDate startDate;
    QDate endDate;
    BacktestMetrics metrics;
    QVector<double> equityCurve;
    QVector<double> drawdownCurve;
    QVector<double> rollingYield;
    QVector<QDate> dates;
    QDateTime computedAt;
};

Q_DECLARE_METATYPE(BacktestMetrics)
Q_DECLARE_METATYPE(BacktestResult)
