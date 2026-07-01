#include "CsvParser.h"

#include <algorithm>

#include <QFile>
#include <QHash>
#include <QRegularExpression>
#include <QSet>

#include "../../models/Asset.h"
#include "../data/DataStore.h"

namespace {
constexpr double kDefaultYield = 0.02;
constexpr double kDefaultBeta = 1.0;
constexpr double kWeightTolerance = 1e-9;

struct ParsedRow {
    int rowNumber = 0;
    QString ticker;
    double amount = 0.0;
    double price = 0.0;
    double dividendYield = kDefaultYield;
    double beta = kDefaultBeta;
};

QString normalizeTicker(const QString& ticker)
{
    return ticker.trimmed().toUpper();
}

bool looksLikeHeader(const QStringList& columns)
{
    for (const auto& column : columns) {
        const QString lowered = column.trimmed().toLower();
        if (lowered.contains(QStringLiteral("ticker")) ||
            lowered.contains(QStringLiteral("symbol")) ||
            lowered.contains(QStringLiteral("shares")) ||
            lowered.contains(QStringLiteral("weight"))) {
            return true;
        }
    }

    return false;
}

QChar detectDelimiter(const QString& line)
{
    const int commaCount = line.count(QLatin1Char(','));
    const int tabCount = line.count(QLatin1Char('\t'));
    return tabCount > commaCount ? QLatin1Char('\t') : QLatin1Char(',');
}

QStringList splitLine(const QString& line, QChar delimiter)
{
    return line.split(delimiter, Qt::KeepEmptyParts);
}

bool isValidTicker(const QString& ticker)
{
    static const QRegularExpression pattern(QStringLiteral("^[A-Z][A-Z.-]{0,9}$"));
    return pattern.match(ticker).hasMatch();
}

void normalizeWeights(QVector<Holding>& holdings)
{
    double totalWeight = 0.0;
    for (const auto& holding : holdings) {
        totalWeight += holding.weight;
    }

    if (totalWeight <= 0.0) {
        return;
    }

    for (auto& holding : holdings) {
        holding.weight /= totalWeight;
    }
}

QHash<QString, Asset> loadUniverse()
{
    QHash<QString, Asset> universe;
    const DataStore dataStore;
    const auto assets = dataStore.loadAssets();
    for (const auto& asset : assets) {
        universe.insert(normalizeTicker(asset.ticker), asset);
    }
    return universe;
}
}

ParseResult CsvParser::parse(const QString& filePath) const
{
    ParseResult result;
    QFile file(filePath);
    if (!file.exists()) {
        result.errors.append(QStringLiteral("File not found: %1").arg(filePath));
        return result;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.errors.append(QStringLiteral("Could not open file: %1").arg(filePath));
        return result;
    }

    return parseText(QString::fromUtf8(file.readAll()));
}

ParseResult CsvParser::parseText(const QString& csvText) const
{
    if (csvText.trimmed().isEmpty()) {
        ParseResult result;
        result.errors.append(QStringLiteral("CSV content is empty."));
        return result;
    }

    return processLines(csvText.split(QRegularExpression(QStringLiteral("\\r\\n|\\n|\\r")), Qt::KeepEmptyParts));
}

ParseResult CsvParser::processLines(const QStringList& lines) const
{
    ParseResult result;

    int firstContentIndex = -1;
    for (int i = 0; i < lines.size(); ++i) {
        if (!lines[i].trimmed().isEmpty()) {
            firstContentIndex = i;
            break;
        }
    }

    if (firstContentIndex < 0) {
        result.errors.append(QStringLiteral("CSV content is empty."));
        return result;
    }

    const QHash<QString, Asset> universe = loadUniverse();
    const QChar delimiter = detectDelimiter(lines[firstContentIndex]);
    const QStringList firstColumns = splitLine(lines[firstContentIndex], delimiter);
    const bool hasHeader = looksLikeHeader(firstColumns);

    int tickerIndex = 0;
    int valueIndex = 1;
    bool hasExplicitWeight = false;
    bool hasExplicitShares = false;
    int dataStartIndex = firstContentIndex;

    if (hasHeader) {
        dataStartIndex = firstContentIndex + 1;
        for (int i = 0; i < firstColumns.size(); ++i) {
            const QString lowered = firstColumns[i].trimmed().toLower();
            if (lowered.contains(QStringLiteral("ticker")) || lowered.contains(QStringLiteral("symbol"))) {
                tickerIndex = i;
            } else if (lowered.contains(QStringLiteral("weight"))) {
                valueIndex = i;
                hasExplicitWeight = true;
            } else if (lowered.contains(QStringLiteral("shares"))) {
                valueIndex = i;
                hasExplicitShares = true;
            }
        }

        if ((!hasExplicitWeight && !hasExplicitShares) || tickerIndex == valueIndex) {
            result.errors.append(QStringLiteral("Header must include ticker/symbol and weight/shares columns."));
            return result;
        }
    }

    QVector<ParsedRow> parsedRows;
    QSet<QString> warnedTickers;

    for (int lineIndex = dataStartIndex; lineIndex < lines.size(); ++lineIndex) {
        const QString rawLine = lines[lineIndex].trimmed();
        if (rawLine.isEmpty()) {
            continue;
        }

        const QStringList columns = splitLine(lines[lineIndex], delimiter);
        if (columns.size() <= qMax(tickerIndex, valueIndex)) {
            result.errors.append(QStringLiteral("Row %1: expected at least two columns.").arg(lineIndex + 1));
            continue;
        }

        const QString ticker = normalizeTicker(columns[tickerIndex]);
        if (ticker.isEmpty()) {
            result.errors.append(QStringLiteral("Row %1: empty ticker.").arg(lineIndex + 1));
            continue;
        }

        if (!isValidTicker(ticker)) {
            result.errors.append(QStringLiteral("Row %1: invalid ticker '%2'.").arg(lineIndex + 1).arg(ticker));
            continue;
        }

        bool ok = false;
        const double amount = columns[valueIndex].trimmed().toDouble(&ok);
        if (!ok) {
            result.errors.append(QStringLiteral("Row %1: invalid numeric value '%2'.").arg(lineIndex + 1).arg(columns[valueIndex].trimmed()));
            continue;
        }

        if (amount < 0.0) {
            result.errors.append(QStringLiteral("Row %1: negative values are not allowed.").arg(lineIndex + 1));
            continue;
        }

        ParsedRow row;
        row.rowNumber = lineIndex + 1;
        row.ticker = ticker;
        row.amount = amount;

        const auto assetIt = universe.constFind(ticker);
        if (assetIt != universe.constEnd()) {
            row.price = assetIt->price;
            row.dividendYield = assetIt->dividendYield;
            row.beta = assetIt->beta;
        } else if (!warnedTickers.contains(ticker)) {
            warnedTickers.insert(ticker);
            result.warnings.append(QStringLiteral("Ticker '%1' not in universe, using defaults").arg(ticker));
        }

        parsedRows.append(row);
    }

    if (parsedRows.isEmpty()) {
        if (result.errors.isEmpty()) {
            result.errors.append(QStringLiteral("No holdings found in CSV content."));
        }
        return result;
    }

    const bool treatAsShares = hasExplicitShares || (!hasExplicitWeight && [&parsedRows]() {
        for (const auto& row : parsedRows) {
            if (row.amount > 1.0 + kWeightTolerance) {
                return true;
            }
        }
        return false;
    }());

    if (!treatAsShares) {
        for (const auto& row : parsedRows) {
            if (row.amount > 1.0 + kWeightTolerance) {
                result.errors.append(QStringLiteral("Row %1: weight must be less than or equal to 1.0.").arg(row.rowNumber));
            }
        }
    }

    if (!result.errors.isEmpty()) {
        return result;
    }

    QVector<Holding> holdings;
    holdings.reserve(parsedRows.size());

    if (treatAsShares) {
        const bool hasAnyPrice = std::any_of(parsedRows.cbegin(), parsedRows.cend(), [](const ParsedRow& row) {
            return row.price > 0.0;
        });

        if (!hasAnyPrice) {
            const double equalWeight = 1.0 / static_cast<double>(parsedRows.size());
            for (const auto& row : parsedRows) {
                holdings.append({row.ticker, equalWeight, row.dividendYield, row.beta});
            }
        } else {
            double totalValue = 0.0;
            for (const auto& row : parsedRows) {
                totalValue += row.amount * (row.price > 0.0 ? row.price : 1.0);
            }

            if (totalValue <= 0.0) {
                result.errors.append(QStringLiteral("Share values could not be converted to weights."));
                return result;
            }

            for (const auto& row : parsedRows) {
                const double effectivePrice = row.price > 0.0 ? row.price : 1.0;
                holdings.append({row.ticker, (row.amount * effectivePrice) / totalValue, row.dividendYield, row.beta});
            }
        }
    } else {
        double totalWeight = 0.0;
        for (const auto& row : parsedRows) {
            totalWeight += row.amount;
        }

        if (totalWeight <= 0.0) {
            result.errors.append(QStringLiteral("Total weight must be greater than zero."));
            return result;
        }

        for (const auto& row : parsedRows) {
            holdings.append({row.ticker, row.amount / totalWeight, row.dividendYield, row.beta});
        }
    }

    QVector<Holding> aggregated;
    QHash<QString, int> indicesByTicker;
    for (const auto& holding : holdings) {
        const auto existing = indicesByTicker.constFind(holding.ticker);
        if (existing == indicesByTicker.constEnd()) {
            indicesByTicker.insert(holding.ticker, aggregated.size());
            aggregated.append(holding);
            continue;
        }

        aggregated[*existing].weight += holding.weight;
    }

    normalizeWeights(aggregated);
    result.holdings = aggregated;
    return result;
}
