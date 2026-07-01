#pragma once



#include <QDateTime>

#include <QVector>



#include "Portfolio.h"



struct SensitivityPoint {

    double yield = 0.0;

    double beta = 0.0;

    double achievedYield = 0.0;

    QVector<Holding> holdings;

};



struct SensitivityCurve {

    double minYield = 0.0;

    double maxYield = 0.0;

    double step = 0.0;

    QVector<SensitivityPoint> points;

    QDateTime computedAt;

};

