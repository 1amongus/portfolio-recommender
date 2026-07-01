#include "CurveGenerator.h"



#include <QtMath>



#include "../../utils/Math.h"



CurveGenerator::CurveGenerator(Optimizer* optimizer, QObject* parent)

    : QObject(parent)

    , m_optimizer(optimizer)

{

}



SensitivityCurve CurveGenerator::generate(double minYield, double maxYield, double step) const

{

    SensitivityCurve curve;

    curve.minYield = minYield;

    curve.maxYield = maxYield;

    curve.step = step;

    curve.computedAt = QDateTime::currentDateTimeUtc();



    if (m_optimizer == nullptr || step <= 0.0 || maxYield < minYield) {

        return curve;

    }



    const double epsilon = step * 0.5;

    const int total = qMax(1, static_cast<int>(qFloor(((maxYield - minYield) / step) + 1e-9)) + 1);



    for (int current = 0; current < total; ++current) {

        const double targetYield = minYield + (static_cast<double>(current) * step);

        if (targetYield > maxYield + epsilon) {

            break;

        }



        const auto holdings = m_optimizer->optimize(targetYield, 5);

        if (!holdings.isEmpty()) {

            SensitivityPoint point;

            point.yield = targetYield;

            point.beta = portfolioBeta(holdings);

            point.achievedYield = portfolioYield(holdings);

            point.holdings = holdings;

            curve.points.append(point);

        }



        emit const_cast<CurveGenerator*>(this)->progressChanged(current + 1, total);

    }



    return curve;

}

