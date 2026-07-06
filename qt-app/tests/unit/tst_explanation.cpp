#include <QtTest>

#include "../../src/models/BacktestResult.h"
#include "../../src/services/slm/ExplanationEngine.h"

class ExplanationEngineTest : public QObject
{
    Q_OBJECT

private slots:
    void portfolioExplanationContainsTickers();
    void backtestExplanationIncludesMetricValues();
    void betaDescriptions_data();
    void betaDescriptions();
};

void ExplanationEngineTest::portfolioExplanationContainsTickers()
{
    ExplanationEngine engine;
    const QVector<Holding> holdings = {
        {QStringLiteral("SCHD"), 0.60, 0.036, 0.88},
        {QStringLiteral("JNJ"), 0.40, 0.032, 0.54}
    };

    const QString explanation = engine.explainPortfolio(holdings, 0.033, 0.0344, 0.74);

    QVERIFY(!explanation.trimmed().isEmpty());
    QVERIFY(explanation.contains(QStringLiteral("SCHD")));
    QVERIFY(explanation.contains(QStringLiteral("JNJ")));
}

void ExplanationEngineTest::backtestExplanationIncludesMetricValues()
{
    ExplanationEngine engine;
    BacktestMetrics metrics;
    metrics.totalReturn = 0.125;
    metrics.annualizedReturn = 0.04;
    metrics.maxDrawdown = -0.09;
    metrics.sharpeRatio = 1.23;
    metrics.sortinoRatio = 1.78;
    metrics.averageYield = 0.035;
    metrics.yieldStability = 0.004;
    metrics.portfolioBeta = 0.81;

    const QString explanation = engine.explainBacktest(metrics, 3);

    QVERIFY(explanation.contains(QStringLiteral("3-year")));
    QVERIFY(explanation.contains(QStringLiteral("12.50%")));
    QVERIFY(explanation.contains(QStringLiteral("4.00%")));
    QVERIFY(explanation.contains(QStringLiteral("1.23")));
    QVERIFY(explanation.contains(QStringLiteral("1.78")));
    QVERIFY(explanation.contains(QStringLiteral("9.00%")));
}

void ExplanationEngineTest::betaDescriptions_data()
{
    QTest::addColumn<double>("beta");
    QTest::addColumn<QString>("description");

    QTest::newRow("very-low") << 0.40 << QStringLiteral("very low");
    QTest::newRow("moderate") << 0.65 << QStringLiteral("moderate");
    QTest::newRow("near-market") << 0.95 << QStringLiteral("near-market");
    QTest::newRow("high") << 1.10 << QStringLiteral("high");
}

void ExplanationEngineTest::betaDescriptions()
{
    QFETCH(double, beta);
    QFETCH(QString, description);

    ExplanationEngine engine;
    const QVector<Holding> holdings = {
        {QStringLiteral("TEST"), 1.0, 0.03, beta}
    };

    const QString explanation = engine.explainPortfolio(holdings, 0.03, 0.03, beta);
    QVERIFY(explanation.contains(description));
}

QTEST_MAIN(ExplanationEngineTest)
#include "tst_explanation.moc"
