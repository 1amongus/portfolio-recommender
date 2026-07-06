#include "ImportController.h"

#include <QDate>
#include <QDateTime>
#include <QFile>
#include <QTime>
#include <QtGlobal>
#include <QRegularExpression>

#include "../../utils/Math.h"

namespace {
constexpr double kDefaultYield = 0.02;
constexpr double kDefaultBeta = 1.0;

bool isValidTicker(const QString& ticker)
{
    static const QRegularExpression pattern(QStringLiteral("^[A-Z][A-Z.-]{0,9}$"));
    return pattern.match(ticker).hasMatch();
}

QVariantMap metricsToVariantMap(const BacktestMetrics& metrics)
{
    return {
        {QStringLiteral("totalReturn"), metrics.totalReturn},
        {QStringLiteral("annualizedReturn"), metrics.annualizedReturn},
        {QStringLiteral("maxDrawdown"), metrics.maxDrawdown},
        {QStringLiteral("sharpeRatio"), metrics.sharpeRatio},
        {QStringLiteral("sortinoRatio"), metrics.sortinoRatio},
        {QStringLiteral("averageYield"), metrics.averageYield},
        {QStringLiteral("yieldStability"), metrics.yieldStability},
        {QStringLiteral("portfolioBeta"), metrics.portfolioBeta}
    };
}

QVariantList curveToVariantList(const QVector<QDate>& dates, const QVector<double>& values)
{
    QVariantList data;
    const qsizetype count = qMin(dates.size(), values.size());
    data.reserve(count);
    for (qsizetype index = 0; index < count; ++index) {
        data.append(QVariantMap{
            {QStringLiteral("x"), QDateTime(dates[index], QTime(0, 0), Qt::UTC).toMSecsSinceEpoch()},
            {QStringLiteral("y"), values[index]}
        });
    }
    return data;
}

QVariantMap backtestToVariantMap(const BacktestResult& result)
{
    return {
        {QStringLiteral("portfolioId"), result.portfolioId},
        {QStringLiteral("startDate"), result.startDate.toString(Qt::ISODate)},
        {QStringLiteral("endDate"), result.endDate.toString(Qt::ISODate)},
        {QStringLiteral("metrics"), metricsToVariantMap(result.metrics)},
        {QStringLiteral("equityCurveData"), curveToVariantList(result.dates, result.equityCurve)},
        {QStringLiteral("drawdownCurveData"), curveToVariantList(result.dates, result.drawdownCurve)},
        {QStringLiteral("rollingYieldData"), curveToVariantList(result.dates, result.rollingYield)}
    };
}
}

ImportController::ImportController(QObject* parent)
    : QObject(parent)
    , m_targetYield(0.03)
    , m_isLoading(false)
    , m_optimizer(&m_dataStore)
    , m_backtestEngine(&m_dataStore)
{
}

QVariantList ImportController::importedHoldings() const
{
    return m_importedHoldings;
}

QVariantList ImportController::rebalancedHoldings() const
{
    return m_rebalancedHoldings;
}

QVariantMap ImportController::comparison() const
{
    return m_comparison;
}

QVariantMap ImportController::originalBacktest() const
{
    return m_originalBacktest;
}

QVariantMap ImportController::rebalancedBacktest() const
{
    return m_rebalancedBacktest;
}

double ImportController::targetYield() const
{
    return m_targetYield;
}

bool ImportController::isLoading() const
{
    return m_isLoading;
}

QString ImportController::errorMessage() const
{
    return m_errorMessage;
}

QStringList ImportController::parseWarnings() const
{
    return m_parseWarnings;
}

void ImportController::setTargetYield(double targetYield)
{
    if (qFuzzyCompare(m_targetYield + 1.0, targetYield + 1.0)) {
        return;
    }

    m_targetYield = targetYield;
    emit targetYieldChanged();
}

void ImportController::importFromFile(const QString& filePath)
{
    if (m_isLoading) {
        return;
    }

    const QString trimmedPath = filePath.trimmed();
    if (trimmedPath.isEmpty()) {
        setErrorMessage(QStringLiteral("Please provide a CSV file path."));
        return;
    }

    QFile file(trimmedPath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setErrorMessage(QStringLiteral("Could not open file: %1").arg(trimmedPath));
        return;
    }
    file.close();

    setIsLoading(true);
    setErrorMessage(QString());
    setParseWarnings({});

    applyParseResult(m_parser.parse(trimmedPath));
    setIsLoading(false);
}

void ImportController::importFromText(const QString& csvText)
{
    if (m_isLoading) {
        return;
    }

    setIsLoading(true);
    setErrorMessage(QString());
    setParseWarnings({});

    applyParseResult(m_parser.parseText(csvText));
    setIsLoading(false);
}

void ImportController::rebalance()
{
    if (m_isLoading) {
        return;
    }

    if (m_importedHoldingsData.isEmpty()) {
        setErrorMessage(QStringLiteral("Import or add holdings before rebalancing."));
        return;
    }

    setIsLoading(true);
    setErrorMessage(QString());

    const QVector<Holding> original = normalizedHoldings(m_importedHoldingsData);
    const int maxAssets = qMax(1, m_importedHoldingsData.size());
    const QVector<Holding> rebalanced = m_optimizer.optimize(m_targetYield, maxAssets);

    if (rebalanced.isEmpty()) {
        setRebalancedHoldingsData({});
        setComparisonData({});
        setOriginalBacktestData({});
        setRebalancedBacktestData({});
        setErrorMessage(QStringLiteral("No qualifying assets available for the requested yield."));
        setIsLoading(false);
        return;
    }

    setRebalancedHoldingsData(rebalanced);
    setComparisonData(buildComparison(original, rebalanced));
    setOriginalBacktestData({});
    setRebalancedBacktestData({});
    setIsLoading(false);
}

void ImportController::addHolding(const QString& ticker, double weight)
{
    const QString normalizedTicker = ticker.trimmed().toUpper();
    if (normalizedTicker.isEmpty()) {
        setErrorMessage(QStringLiteral("Ticker is required."));
        return;
    }

    if (!isValidTicker(normalizedTicker)) {
        setErrorMessage(QStringLiteral("Invalid ticker format."));
        return;
    }

    if (qIsNaN(weight) || weight < 0.0 || weight > 1.0) {
        setErrorMessage(QStringLiteral("Weight must be between 0.0 and 1.0."));
        return;
    }

    bool foundInUniverse = false;
    m_importedHoldingsData.append(buildHolding(normalizedTicker, weight, &foundInUniverse));
    setImportedHoldingsData(m_importedHoldingsData);
    setRebalancedHoldingsData({});
    setComparisonData({});
    setOriginalBacktestData({});
    setRebalancedBacktestData({});
    setErrorMessage(QString());

    if (!foundInUniverse) {
        QStringList warnings = m_parseWarnings;
        const QString warning = QStringLiteral("Ticker '%1' not in universe, using defaults").arg(normalizedTicker);
        if (!warnings.contains(warning)) {
            warnings.append(warning);
            setParseWarnings(warnings);
        }
    }
}

void ImportController::removeHolding(int index)
{
    if (index < 0 || index >= m_importedHoldingsData.size()) {
        setErrorMessage(QStringLiteral("Holding index is out of range."));
        return;
    }

    m_importedHoldingsData.removeAt(index);
    setImportedHoldingsData(m_importedHoldingsData);
    setRebalancedHoldingsData({});
    setComparisonData({});
    setOriginalBacktestData({});
    setRebalancedBacktestData({});
    setErrorMessage(QString());
}

void ImportController::clear()
{
    setImportedHoldingsData({});
    setRebalancedHoldingsData({});
    setComparisonData({});
    setOriginalBacktestData({});
    setRebalancedBacktestData({});
    setParseWarnings({});
    setErrorMessage(QString());
}

void ImportController::runComparativeBacktest(int years)
{
    if (m_isLoading) {
        return;
    }

    if (years <= 0) {
        setErrorMessage(QStringLiteral("Please choose a valid back-test duration."));
        return;
    }

    const QVector<Holding> original = normalizedHoldings(m_importedHoldingsData);
    const QVector<Holding> rebalanced = normalizedHoldings(m_rebalancedHoldingsData);
    if (original.isEmpty() || rebalanced.isEmpty()) {
        setOriginalBacktestData({});
        setRebalancedBacktestData({});
        setErrorMessage(QStringLiteral("Import and rebalance holdings before running a comparison back-test."));
        return;
    }

    setIsLoading(true);
    setErrorMessage(QString());

    const QDate endDate = QDate::currentDate();
    const QDate startDate = endDate.addYears(-years);
    const BacktestResult originalResult = m_backtestEngine.run(original, startDate, endDate, RebalanceFrequency::Never);
    const BacktestResult rebalancedResult = m_backtestEngine.run(rebalanced, startDate, endDate, RebalanceFrequency::Never);

    setIsLoading(false);

    if (originalResult.equityCurve.isEmpty() || rebalancedResult.equityCurve.isEmpty()) {
        setOriginalBacktestData({});
        setRebalancedBacktestData({});
        setErrorMessage(QStringLiteral("Unable to generate comparative back-tests for the selected portfolios."));
        return;
    }

    setOriginalBacktestData(backtestToVariantMap(originalResult));
    setRebalancedBacktestData(backtestToVariantMap(rebalancedResult));
}

QVariantList ImportController::holdingsToVariantList(const QVector<Holding>& holdings) const
{
    QVariantList result;
    result.reserve(holdings.size());
    for (const auto& holding : holdings) {
        QVariantMap item;
        item.insert(QStringLiteral("ticker"), holding.ticker);
        item.insert(QStringLiteral("weight"), holding.weight);
        item.insert(QStringLiteral("yield"), holding.yield);
        item.insert(QStringLiteral("beta"), holding.beta);
        result.append(item);
    }
    return result;
}

QVariantMap ImportController::buildComparison(const QVector<Holding>& original, const QVector<Holding>& rebalanced) const
{
    const double originalBeta = portfolioBeta(original);
    const double rebalancedBeta = portfolioBeta(rebalanced);
    const double originalYield = portfolioYield(original);
    const double rebalancedYield = portfolioYield(rebalanced);

    return {
        {QStringLiteral("originalBeta"), originalBeta},
        {QStringLiteral("rebalancedBeta"), rebalancedBeta},
        {QStringLiteral("betaReduction"), originalBeta - rebalancedBeta},
        {QStringLiteral("originalYield"), originalYield},
        {QStringLiteral("rebalancedYield"), rebalancedYield},
        {QStringLiteral("yieldDelta"), rebalancedYield - originalYield}
    };
}

QVector<Holding> ImportController::normalizedHoldings(const QVector<Holding>& holdings) const
{
    QVector<Holding> normalized = holdings;
    double totalWeight = 0.0;
    for (const auto& holding : normalized) {
        totalWeight += holding.weight;
    }

    if (totalWeight <= 0.0) {
        return {};
    }

    for (auto& holding : normalized) {
        holding.weight /= totalWeight;
    }

    return normalized;
}

Holding ImportController::buildHolding(const QString& ticker, double weight, bool* foundInUniverse) const
{
    if (foundInUniverse != nullptr) {
        *foundInUniverse = false;
    }

    const auto assets = m_dataStore.loadAssets();
    for (const auto& asset : assets) {
        if (asset.ticker.trimmed().compare(ticker, Qt::CaseInsensitive) == 0) {
            if (foundInUniverse != nullptr) {
                *foundInUniverse = true;
            }
            return {ticker, weight, asset.dividendYield, asset.beta};
        }
    }

    return {ticker, weight, kDefaultYield, kDefaultBeta};
}

void ImportController::applyParseResult(const ParseResult& result)
{
    setImportedHoldingsData(result.holdings);
    setRebalancedHoldingsData({});
    setComparisonData({});
    setParseWarnings(result.warnings);
    setOriginalBacktestData({});
    setRebalancedBacktestData({});

    if (!result.errors.isEmpty()) {
        setErrorMessage(result.errors.join(QLatin1Char('\n')));
        return;
    }

    if (result.holdings.isEmpty()) {
        setErrorMessage(QStringLiteral("No holdings found in CSV content."));
        return;
    }

    setErrorMessage(QString());
}

void ImportController::setImportedHoldingsData(const QVector<Holding>& holdings)
{
    m_importedHoldingsData = holdings;
    m_importedHoldings = holdingsToVariantList(holdings);
    emit importedHoldingsChanged();
}

void ImportController::setRebalancedHoldingsData(const QVector<Holding>& holdings)
{
    m_rebalancedHoldingsData = holdings;
    m_rebalancedHoldings = holdingsToVariantList(holdings);
    emit rebalancedHoldingsChanged();
}

void ImportController::setComparisonData(const QVariantMap& comparison)
{
    m_comparison = comparison;
    emit comparisonChanged();
}

void ImportController::setOriginalBacktestData(const QVariantMap& backtest)
{
    m_originalBacktest = backtest;
    emit originalBacktestChanged();
}

void ImportController::setRebalancedBacktestData(const QVariantMap& backtest)
{
    m_rebalancedBacktest = backtest;
    emit rebalancedBacktestChanged();
}

void ImportController::setIsLoading(bool isLoading)
{
    if (m_isLoading == isLoading) {
        return;
    }

    m_isLoading = isLoading;
    emit isLoadingChanged();
}

void ImportController::setErrorMessage(const QString& message)
{
    if (m_errorMessage == message) {
        return;
    }

    m_errorMessage = message;
    emit errorMessageChanged();
}

void ImportController::setParseWarnings(const QStringList& warnings)
{
    if (m_parseWarnings == warnings) {
        return;
    }

    m_parseWarnings = warnings;
    emit parseWarningsChanged();
}
