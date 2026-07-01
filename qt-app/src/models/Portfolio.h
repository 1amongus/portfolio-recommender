#pragma once

#include <QDateTime>
#include <QString>
#include <QVector>

struct Holding {
    QString ticker;
    double weight = 0.0;
    double yield = 0.0;
    double beta = 0.0;
};

struct Portfolio {
    QString id;
    QString name;
    double targetYield = 0.0;
    double aggregateBeta = 0.0;
    double achievedYield = 0.0;
    QVector<Holding> holdings;
    QDateTime createdAt;
};