#include <QtTest>
#include <QStandardPaths>

#include "../../src/services/data/DataStore.h"
#include "../../src/services/import/CsvParser.h"

class CsvParserTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void parsesTickerWeightFormat();
    void parsesTickerSharesFormat();
    void detectsHeaderRow();
    void warnsForUnknownTickers();
    void normalizesWeights();
    void emptyFileReturnsError();
};

void CsvParserTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);

    QCoreApplication::setApplicationName(QStringLiteral("CsvParserTests"));
    QCoreApplication::setOrganizationName(QStringLiteral("PortfolioRecommenderTests"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("portfoliorecommender.local"));

    DataStore store;
    QVERIFY(store.saveAssets({
        {QStringLiteral("AAA"), QStringLiteral("Asset AAA"), QStringLiteral("ETF"), 100.0, 0.03, 0.8, 1000.0, true, QDateTime::currentDateTimeUtc()},
        {QStringLiteral("BBB"), QStringLiteral("Asset BBB"), QStringLiteral("ETF"), 50.0, 0.04, 0.6, 1000.0, true, QDateTime::currentDateTimeUtc()},
        {QStringLiteral("CCC"), QStringLiteral("Asset CCC"), QStringLiteral("ETF"), 25.0, 0.02, 1.1, 1000.0, true, QDateTime::currentDateTimeUtc()}
    }));
}

void CsvParserTest::parsesTickerWeightFormat()
{
    CsvParser parser;
    const auto result = parser.parseText(QStringLiteral("ticker,weight\nAAA,0.60\nBBB,0.40\n"));

    QVERIFY(result.errors.isEmpty());
    QCOMPARE(result.holdings.size(), 2);
    QCOMPARE(result.holdings[0].ticker, QStringLiteral("AAA"));
    QCOMPARE(result.holdings[1].ticker, QStringLiteral("BBB"));
    QVERIFY(qAbs(result.holdings[0].weight - 0.60) < 0.0001);
    QVERIFY(qAbs(result.holdings[1].weight - 0.40) < 0.0001);
}

void CsvParserTest::parsesTickerSharesFormat()
{
    CsvParser parser;
    const auto result = parser.parseText(QStringLiteral("ticker,shares\nAAA,2\nBBB,1\n"));

    QVERIFY(result.errors.isEmpty());
    QCOMPARE(result.holdings.size(), 2);
    QVERIFY(qAbs(result.holdings[0].weight - 0.8) < 0.0001);
    QVERIFY(qAbs(result.holdings[1].weight - 0.2) < 0.0001);
}

void CsvParserTest::detectsHeaderRow()
{
    CsvParser parser;
    const auto result = parser.parseText(QStringLiteral("symbol\tweight\nAAA\t0.25\nBBB\t0.75\n"));

    QVERIFY(result.errors.isEmpty());
    QCOMPARE(result.holdings.size(), 2);
    QVERIFY(qAbs(result.holdings[0].weight - 0.25) < 0.0001);
    QVERIFY(qAbs(result.holdings[1].weight - 0.75) < 0.0001);
}

void CsvParserTest::warnsForUnknownTickers()
{
    CsvParser parser;
    const auto result = parser.parseText(QStringLiteral("ticker,weight\nZZTOP,1.0\n"));

    QVERIFY(result.errors.isEmpty());
    QCOMPARE(result.holdings.size(), 1);
    QCOMPARE(result.warnings.size(), 1);
    QCOMPARE(result.holdings[0].yield, 0.02);
    QCOMPARE(result.holdings[0].beta, 1.0);
}

void CsvParserTest::normalizesWeights()
{
    CsvParser parser;
    const auto result = parser.parseText(QStringLiteral("AAA,0.2\nBBB,0.3\nCCC,0.5\n"));

    QVERIFY(result.errors.isEmpty());
    QCOMPARE(result.holdings.size(), 3);

    double totalWeight = 0.0;
    for (const auto& holding : result.holdings) {
        totalWeight += holding.weight;
    }
    QVERIFY(qAbs(totalWeight - 1.0) < 0.0001);
}

void CsvParserTest::emptyFileReturnsError()
{
    CsvParser parser;
    const auto result = parser.parseText(QString());

    QVERIFY(result.holdings.isEmpty());
    QVERIFY(!result.errors.isEmpty());
}

QTEST_MAIN(CsvParserTest)
#include "tst_csvparser.moc"
