#pragma once

#include <QDateTime>
#include <QString>

struct Asset {
    QString ticker;
    QString name;
    QString sector;
    double price = 0.0;
    double dividendYield = 0.0;
    double beta = 0.0;
    double marketCap = 0.0;
    bool isETF = false;
    QDateTime lastUpdated;
};