#include <QtTest>

#include "../../src/services/data/DataStore.h"
#include "../../src/services/optimization/Optimizer.h"

class TestDataStore : public DataStore
{
public:
    explicit TestDataStore(QVector<Asset> assets)
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

class OptimizerTest : public QObject
{
    Q_OBJECT

private slots:
    void emptyUniverseReturnsEmptyResult();
    void singleQualifyingAssetIsSelected();
    void maxAssetsConstraintIsRespected();
};

void OptimizerTest::emptyUniverseReturnsEmptyResult()
{
    TestDataStore store({});
    Optimizer optimizer(&store);
    QVERIFY(optimizer.optimize(0.03).isEmpty());
}

void OptimizerTest::singleQualifyingAssetIsSelected()
{
    TestDataStore store({
        {QStringLiteral("LOW"), QStringLiteral("Low Yield"), QStringLiteral("ETF"), 10.0, 0.01, 0.3, 100.0, true, QDateTime::currentDateTimeUtc()},
        {QStringLiteral("HIGH"), QStringLiteral("High Yield"), QStringLiteral("ETF"), 10.0, 0.05, 0.4, 100.0, true, QDateTime::currentDateTimeUtc()}
    });
    Optimizer optimizer(&store);

    const auto result = optimizer.optimize(0.04, 5);
    QCOMPARE(result.size(), 1);
    QCOMPARE(result.first().ticker, QStringLiteral("HIGH"));
    QCOMPARE(result.first().weight, 1.0);
}

void OptimizerTest::maxAssetsConstraintIsRespected()
{
    QVector<Asset> assets;
    for (int i = 0; i < 10; ++i) {
        assets.append({QStringLiteral("A%1").arg(i), QStringLiteral("Asset %1").arg(i), QStringLiteral("ETF"), 100.0, 0.03 + i * 0.001, 0.2 + i * 0.05, 1000.0, true, QDateTime::currentDateTimeUtc()});
    }

    TestDataStore store(assets);
    Optimizer optimizer(&store);
    const auto result = optimizer.optimize(0.08, 5);
    QVERIFY(result.size() <= 5);
}

QTEST_MAIN(OptimizerTest)
#include "tst_optimizer.moc"