#include <QtTest>

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUuid>

#include "../../src/models/BacktestResult.h"
#include "../../src/models/Portfolio.h"
#include "../../src/services/export/CsvExporter.h"
#include "../../src/services/export/PdfExporter.h"

class ExportTest : public QObject
{
    Q_OBJECT

private slots:
    void csvExportProducesExpectedHeaders();
    void pdfExportCreatesNonEmptyFile();
    void emptyPortfolioExportDoesNotCrash();
};

namespace {
QString uniquePath(const QString& suffix)
{
    return QDir::current().filePath(QStringLiteral("%1_%2").arg(QUuid::createUuid().toString(QUuid::WithoutBraces), suffix));
}

Portfolio samplePortfolio()
{
    Portfolio portfolio;
    portfolio.id = QStringLiteral("portfolio-1");
    portfolio.name = QStringLiteral("Income Core");
    portfolio.targetYield = 0.035;
    portfolio.achievedYield = 0.0365;
    portfolio.aggregateBeta = 0.82;
    portfolio.createdAt = QDateTime::currentDateTimeUtc();
    portfolio.holdings = {
        {QStringLiteral("SCHD"), 0.6, 0.036, 0.88},
        {QStringLiteral("VYM"), 0.4, 0.031, 0.85}
    };
    return portfolio;
}

BacktestMetrics sampleMetrics()
{
    BacktestMetrics metrics;
    metrics.totalReturn = 0.18;
    metrics.annualizedReturn = 0.057;
    metrics.maxDrawdown = -0.12;
    metrics.sharpeRatio = 1.14;
    metrics.sortinoRatio = 1.49;
    metrics.averageYield = 0.034;
    metrics.yieldStability = 0.92;
    metrics.portfolioBeta = 0.82;
    return metrics;
}
}

void ExportTest::csvExportProducesExpectedHeaders()
{
    const QString path = uniquePath(QStringLiteral("portfolio.csv"));
    QVERIFY(CsvExporter::exportPortfolio(samplePortfolio(), path));

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = QString::fromUtf8(file.readAll());
    QVERIFY(content.startsWith(QStringLiteral("\"Ticker\",\"Weight\",\"Yield\",\"Beta\"")));
    QVERIFY(content.contains(QStringLiteral("\"SCHD\"")));
    QVERIFY(content.contains(QStringLiteral("\"Summary\"")));

    file.close();
    QFile::remove(path);
}

void ExportTest::pdfExportCreatesNonEmptyFile()
{
    const QString path = uniquePath(QStringLiteral("portfolio.pdf"));
    const BacktestMetrics metrics = sampleMetrics();
    QVERIFY(PdfExporter::exportPortfolioReport(samplePortfolio(), &metrics, path));

    QFileInfo fileInfo(path);
    QVERIFY(fileInfo.exists());
    QVERIFY(fileInfo.size() > 0);

    QFile::remove(path);
}

void ExportTest::emptyPortfolioExportDoesNotCrash()
{
    Portfolio portfolio;
    portfolio.name = QStringLiteral("Empty");

    const QString csvPath = uniquePath(QStringLiteral("empty.csv"));
    const QString pdfPath = uniquePath(QStringLiteral("empty.pdf"));

    QVERIFY(CsvExporter::exportPortfolio(portfolio, csvPath));
    QVERIFY(PdfExporter::exportPortfolioReport(portfolio, nullptr, pdfPath));

    QFile::remove(csvPath);
    QFile::remove(pdfPath);
}

QTEST_MAIN(ExportTest)
#include "tst_export.moc"
