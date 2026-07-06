#include "PdfExporter.h"

#include <QDateTime>
#include <QFileInfo>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPdfWriter>

namespace {
constexpr int kMargin = 72;
constexpr int kRowHeight = 28;

QString percentText(double value)
{
    return QStringLiteral("%1%").arg(QString::number(value * 100.0, 'f', 2));
}

void drawFooter(QPainter& painter, QPdfWriter& writer, const QString& footer)
{
    const QRect pageRect = writer.pageLayout().paintRectPixels(writer.resolution());
    const QRect footerRect(kMargin, pageRect.height() - kMargin + 18, pageRect.width() - (kMargin * 2), 24);
    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);
    painter.setPen(QColor(QStringLiteral("#6b7280")));
    painter.drawText(footerRect, Qt::AlignLeft | Qt::AlignVCenter, footer);
    painter.drawText(footerRect, Qt::AlignRight | Qt::AlignVCenter, QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd")));
}

void drawTitle(QPainter& painter, int& y, const QString& title, const QString& subtitle = {})
{
    QFont titleFont = painter.font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.setPen(QColor(QStringLiteral("#111827")));
    painter.drawText(kMargin, y, title);
    y += 34;

    if (!subtitle.isEmpty()) {
        QFont subtitleFont = painter.font();
        subtitleFont.setPointSize(10);
        subtitleFont.setBold(false);
        painter.setFont(subtitleFont);
        painter.setPen(QColor(QStringLiteral("#4b5563")));
        painter.drawText(kMargin, y, subtitle);
        y += 24;
    }
}

void drawKeyValue(QPainter& painter, int& y, const QString& label, const QString& value)
{
    QFont labelFont = painter.font();
    labelFont.setPointSize(10);
    labelFont.setBold(true);
    painter.setFont(labelFont);
    painter.setPen(QColor(QStringLiteral("#1f2937")));
    painter.drawText(kMargin, y, label);

    QFont valueFont = painter.font();
    valueFont.setBold(false);
    painter.setFont(valueFont);
    painter.drawText(kMargin + 160, y, value);
    y += 24;
}

void drawTable(QPainter& painter, int startY, int width, const QStringList& headers, const QVector<QStringList>& rows)
{
    const int columnCount = headers.size();
    if (columnCount <= 0) {
        return;
    }

    const int columnWidth = width / columnCount;
    const int totalRows = rows.size() + 1;
    QRect tableRect(kMargin, startY, width, kRowHeight * totalRows);

    painter.setPen(QColor(QStringLiteral("#9ca3af")));
    painter.setBrush(QColor(QStringLiteral("#e5eef7")));
    painter.drawRect(QRect(kMargin, startY, width, kRowHeight));

    QFont headerFont = painter.font();
    headerFont.setPointSize(10);
    headerFont.setBold(true);
    painter.setFont(headerFont);
    painter.setPen(QColor(QStringLiteral("#111827")));

    for (int column = 0; column < columnCount; ++column) {
        const QRect cellRect(kMargin + (column * columnWidth), startY, columnWidth, kRowHeight);
        painter.drawText(cellRect.adjusted(8, 0, -8, 0), Qt::AlignLeft | Qt::AlignVCenter, headers.at(column));
        painter.drawLine(cellRect.topRight(), cellRect.bottomRight());
    }

    QFont bodyFont = painter.font();
    bodyFont.setBold(false);
    painter.setFont(bodyFont);
    painter.setPen(QColor(QStringLiteral("#374151")));

    for (int row = 0; row < rows.size(); ++row) {
        const int rowTop = startY + kRowHeight + (row * kRowHeight);
        if (row % 2 == 0) {
            painter.fillRect(QRect(kMargin, rowTop, width, kRowHeight), QColor(QStringLiteral("#f8fafc")));
        }

        const auto& values = rows.at(row);
        for (int column = 0; column < columnCount; ++column) {
            const QRect cellRect(kMargin + (column * columnWidth), rowTop, columnWidth, kRowHeight);
            painter.drawText(cellRect.adjusted(8, 0, -8, 0), Qt::AlignLeft | Qt::AlignVCenter, column < values.size() ? values.at(column) : QString());
            painter.drawLine(cellRect.topRight(), cellRect.bottomRight());
        }
    }

    for (int row = 0; row <= totalRows; ++row) {
        const int lineY = startY + (row * kRowHeight);
        painter.drawLine(kMargin, lineY, kMargin + width, lineY);
    }

    painter.drawRect(tableRect);
}
}

bool PdfExporter::exportPortfolioReport(const Portfolio& portfolio, const BacktestMetrics* metrics, const QString& filePath)
{
    QPdfWriter writer(filePath);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageOrientation(QPageLayout::Portrait);
    writer.setResolution(144);
    writer.setTitle(QStringLiteral("Portfolio Report"));

    QPainter painter(&writer);
    if (!painter.isActive()) {
        return false;
    }

    const QRect pageRect = writer.pageLayout().paintRectPixels(writer.resolution());
    const int contentWidth = pageRect.width() - (kMargin * 2);
    int y = kMargin;

    drawTitle(
        painter,
        y,
        QStringLiteral("Portfolio Report"),
        portfolio.name.isEmpty() ? QStringLiteral("Portfolio Recommender") : portfolio.name);

    drawKeyValue(painter, y, QStringLiteral("Portfolio Name"), portfolio.name.isEmpty() ? QStringLiteral("Untitled Portfolio") : portfolio.name);
    drawKeyValue(painter, y, QStringLiteral("Target Yield"), percentText(portfolio.targetYield));
    drawKeyValue(painter, y, QStringLiteral("Achieved Yield"), percentText(portfolio.achievedYield));
    drawKeyValue(painter, y, QStringLiteral("Portfolio Beta"), QString::number(portfolio.aggregateBeta, 'f', 3));
    drawKeyValue(painter, y, QStringLiteral("Generated"), portfolio.createdAt.isValid() ? portfolio.createdAt.toLocalTime().toString(QStringLiteral("yyyy-MM-dd hh:mm")) : QStringLiteral("N/A"));
    y += 12;

    QVector<QStringList> holdingRows;
    holdingRows.reserve(portfolio.holdings.size());
    double totalWeight = 0.0;
    for (const auto& holding : portfolio.holdings) {
        totalWeight += holding.weight;
        holdingRows.append({
            holding.ticker,
            percentText(holding.weight),
            percentText(holding.yield),
            QString::number(holding.beta, 'f', 3)
        });
    }
    holdingRows.append({
        QStringLiteral("Summary"),
        percentText(totalWeight),
        percentText(portfolio.achievedYield),
        QString::number(portfolio.aggregateBeta, 'f', 3)
    });

    drawTable(
        painter,
        y,
        contentWidth,
        {QStringLiteral("Ticker"), QStringLiteral("Weight"), QStringLiteral("Yield"), QStringLiteral("Beta")},
        holdingRows);

    drawFooter(painter, writer, QStringLiteral("Portfolio Recommender"));

    if (metrics != nullptr) {
        writer.newPage();
        y = kMargin;

        drawTitle(painter, y, QStringLiteral("Backtest Metrics"), QStringLiteral("Summary statistics"));
        drawKeyValue(painter, y, QStringLiteral("Total Return"), percentText(metrics->totalReturn));
        drawKeyValue(painter, y, QStringLiteral("Annualized Return"), percentText(metrics->annualizedReturn));
        drawKeyValue(painter, y, QStringLiteral("Max Drawdown"), percentText(metrics->maxDrawdown));
        drawKeyValue(painter, y, QStringLiteral("Sharpe Ratio"), QString::number(metrics->sharpeRatio, 'f', 2));
        drawKeyValue(painter, y, QStringLiteral("Sortino Ratio"), QString::number(metrics->sortinoRatio, 'f', 2));
        drawKeyValue(painter, y, QStringLiteral("Average Yield"), percentText(metrics->averageYield));
        drawKeyValue(painter, y, QStringLiteral("Yield Stability"), QString::number(metrics->yieldStability, 'f', 3));
        drawKeyValue(painter, y, QStringLiteral("Portfolio Beta"), QString::number(metrics->portfolioBeta, 'f', 3));

        drawFooter(painter, writer, QStringLiteral("Portfolio Recommender"));
    }

    painter.end();
    return QFileInfo(filePath).exists() && QFileInfo(filePath).size() > 0;
}
