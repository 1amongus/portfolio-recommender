#include <QtTest>

#include "../../src/utils/Math.h"

class MathTest : public QObject
{
    Q_OBJECT

private slots:
    void betaCalculationWithKnownValues();
    void sharpeRatioCalculation();
    void maxDrawdownCalculation();
};

void MathTest::betaCalculationWithKnownValues()
{
    const QVector<double> assetPrices{100.0, 104.0, 101.92, 108.0352};
    const QVector<double> marketPrices{100.0, 102.0, 100.98, 104.0094};

    const double beta = calculateBeta(assetPrices, marketPrices);
    QVERIFY(qAbs(beta - 2.0) < 0.01);
}

void MathTest::sharpeRatioCalculation()
{
    const QVector<double> returns{0.02, 0.01, 0.03, 0.015};
    const double sharpe = calculateSharpe(returns, 0.005);
    QVERIFY(sharpe > 0.0);
}

void MathTest::maxDrawdownCalculation()
{
    const QVector<double> prices{100.0, 120.0, 90.0, 95.0, 80.0};
    const double drawdown = calculateMaxDrawdown(prices);
    QVERIFY(qAbs(drawdown + 0.3333333) < 0.01);
}

QTEST_MAIN(MathTest)
#include "tst_math.moc"