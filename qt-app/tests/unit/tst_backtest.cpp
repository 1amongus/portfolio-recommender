#include <QtMath>
#include <QtTest>

#include "../../src/services/backtest/Engine.h"
#include "../../src/services/data/DataStore.h"

class BacktestDataStore : public DataStore
{
public:
    explicit BacktestDataStore(QVector<Asset> assets)
        : m_assets(std::move(assets))
    {
    }

    QVector<Asset> loadAssets() const override
    {
        return m_assets;
    }

private:
    QVector<Asset> m_assets;
};

class BacktestEngineTest : public QObject
{
    Q_OBJECT

private slots:
    void emptyHoldingsReturnEmptyResult();
    void singleHoldingProducesValidMetrics();
    void equityCurveMatchesTradingDays();
};

void BacktestEngineTest::emptyHoldingsReturnEmptyResult()
{
    BacktestDataStore store({});
    BacktestEngine engine(&store);

    const BacktestResult result = engine.run({}, QDate(2024, 1, 1), QDate(2024, 12, 31));

    QVERIFY(result.equityCurve.isEmpty());
    QVERIFY(result.drawdownCurve.isEmpty());
    QVERIFY(result.rollingYield.isEmpty());
    QVERIFY(result.dates.isEmpty());
}

void BacktestEngineTest::singleHoldingProducesValidMetrics()
{
    BacktestDataStore store({
        {QStringLiteral("SCHD"), QStringLiteral("Schwab U.S. Dividend Equity ETF"), QStringLiteral("ETF"), 79.0, 0.036, 0.88, 50000000000.0, true, QDateTime::currentDateTimeUtc()}
    });
    BacktestEngine engine(&store);

    const QVector<Holding> holdings = {
        {QStringLiteral("SCHD"), 1.0, 0.036, 0.88}
    };
    const BacktestResult result = engine.run(holdings, QDate(2024, 1, 1), QDate(2024, 12, 31));

    QVERIFY(!result.equityCurve.isEmpty());
    QVERIFY(result.equityCurve.first() > 0.0);
    QVERIFY(qIsFinite(result.metrics.totalReturn));
    QVERIFY(qIsFinite(result.metrics.annualizedReturn));
    QVERIFY(qIsFinite(result.metrics.maxDrawdown));
    QVERIFY(qIsFinite(result.metrics.sharpeRatio));
    QVERIFY(qIsFinite(result.metrics.sortinoRatio));
    QVERIFY(result.metrics.portfolioBeta > 0.0);
}

void BacktestEngineTest::equityCurveMatchesTradingDays()
{
    BacktestDataStore store({
        {QStringLiteral("VYM"), QStringLiteral("Vanguard High Dividend Yield ETF"), QStringLiteral("ETF"), 118.0, 0.031, 0.85, 54000000000.0, true, QDateTime::currentDateTimeUtc()}
    });
    BacktestEngine engine(&store);

    const QDate startDate(2024, 1, 1);
    const QDate endDate(2024, 12, 31);
    const BacktestResult result = engine.run({{QStringLiteral("VYM"), 1.0, 0.031, 0.85}}, startDate, endDate);

    int tradingDays = 0;
    for (QDate date = startDate; date <= endDate; date = date.addDays(1)) {
        if (date.dayOfWeek() >= 1 && date.dayOfWeek() <= 5) {
            ++tradingDays;
        }
    }

    QCOMPARE(result.equityCurve.size(), tradingDays);
    QCOMPARE(result.drawdownCurve.size(), tradingDays);
    QCOMPARE(result.rollingYield.size(), tradingDays);
    QCOMPARE(result.dates.size(), tradingDays);
}

QTEST_MAIN(BacktestEngineTest)
#include "tst_backtest.moc"
