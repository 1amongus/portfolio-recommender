#include "ExportController.h"

#include <QDir>
#include <QFileDialog>
#include <QRegularExpression>
#include <QStandardPaths>

#include "../../services/export/CsvExporter.h"
#include "../../services/export/PdfExporter.h"
#include "BacktestController.h"

ExportController::ExportController(QObject* parent)
    : QObject(parent)
{
}

void ExportController::setBacktestController(BacktestController* backtestController)
{
    m_backtestController = backtestController;
}

void ExportController::exportPortfolioCsv(int portfolioIndex)
{
    const QVector<Portfolio> portfolios = m_dataStore.loadPortfolios();
    if (portfolioIndex < 0 || portfolioIndex >= portfolios.size()) {
        return;
    }

    const Portfolio& portfolio = portfolios.at(portfolioIndex);
    const QString path = getExportPath(
        QStringLiteral("%1.csv").arg(safeDefaultName(portfolio.name.isEmpty() ? QStringLiteral("portfolio") : portfolio.name)),
        QStringLiteral("CSV Files (*.csv)"));
    if (path.isEmpty()) {
        return;
    }

    CsvExporter::exportPortfolio(portfolio, path);
}

void ExportController::exportPortfolioPdf(int portfolioIndex)
{
    const QVector<Portfolio> portfolios = m_dataStore.loadPortfolios();
    if (portfolioIndex < 0 || portfolioIndex >= portfolios.size()) {
        return;
    }

    const Portfolio& portfolio = portfolios.at(portfolioIndex);
    const QString path = getExportPath(
        QStringLiteral("%1.pdf").arg(safeDefaultName(portfolio.name.isEmpty() ? QStringLiteral("portfolio-report") : portfolio.name)),
        QStringLiteral("PDF Files (*.pdf)"));
    if (path.isEmpty()) {
        return;
    }

    PdfExporter::exportPortfolioReport(portfolio, nullptr, path);
}

void ExportController::exportBacktestCsv()
{
    if (m_backtestController == nullptr || !m_backtestController->hasLastResult()) {
        return;
    }

    const QString path = getExportPath(QStringLiteral("backtest-results.csv"), QStringLiteral("CSV Files (*.csv)"));
    if (path.isEmpty()) {
        return;
    }

    CsvExporter::exportBacktest(m_backtestController->lastResult(), path);
}

QString ExportController::getExportPath(const QString& defaultName, const QString& filter)
{
    const QString initialPath = QDir::toNativeSeparators(QDir(defaultExportDirectory()).filePath(safeDefaultName(defaultName)));
    return QFileDialog::getSaveFileName(nullptr, tr("Export File"), initialPath, filter);
}

QString ExportController::defaultExportDirectory() const
{
    const QString documentsDirectory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    return documentsDirectory.isEmpty() ? QDir::homePath() : documentsDirectory;
}

QString ExportController::safeDefaultName(const QString& defaultName) const
{
    QString sanitized = defaultName.trimmed();
    sanitized.replace(QRegularExpression(QStringLiteral("[\\\\/:*?\"<>|]")), QStringLiteral("_"));
    return sanitized.isEmpty() ? QStringLiteral("export") : sanitized;
}
