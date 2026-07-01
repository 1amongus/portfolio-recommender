#pragma once

#include <QVector>

#include "../models/Portfolio.h"

namespace Math {
double calculateBeta(const QVector<double>& prices, const QVector<double>& marketPrices);
double calculateSharpe(const QVector<double>& returns, double riskFreeRate);
double calculateSortino(const QVector<double>& returns, double riskFreeRate);
double calculateMaxDrawdown(const QVector<double>& prices);
double portfolioBeta(const QVector<Holding>& holdings);
double portfolioYield(const QVector<Holding>& holdings);
}

double calculateBeta(const QVector<double>& prices, const QVector<double>& marketPrices);
double calculateSharpe(const QVector<double>& returns, double riskFreeRate);
double calculateSortino(const QVector<double>& returns, double riskFreeRate);
double calculateMaxDrawdown(const QVector<double>& prices);
double portfolioBeta(const QVector<Holding>& holdings);
double portfolioYield(const QVector<Holding>& holdings);