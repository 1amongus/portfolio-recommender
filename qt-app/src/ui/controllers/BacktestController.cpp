#include "BacktestController.h"

#include <QDate>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTime>
#include <QtConcurrent>

#include "../../models/Asset.h"

namespace {
QVariantList toVariantList(const QVector<double>& values)
{
    QVariantList list;
    list.reserve(values.size());
    for (double value : values) {
        list.append(value);
    }
    return list;
}

QVariantList toVariantList(const QVector<QDate>& values)
{
    QVariantList list;
    list.reserve(values.size());
    for (const auto& value : values) {
        list.append(value.toString(Qt::ISODate));
    }
    return list;
}

QVariantMap metricsToVariant(const BacktestMetrics& metrics)
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

QVariantList curveToPointList(const QVector<QDate>& dates, const QVector<double>& values)
{
    QVariantList points;
    const qsizetype count = qMin(dates.size(), values.size());
    points.reserve(count);
    for (qsizetype index = 0; index < count; ++index) {
        points.append(QVariantMap{
            {QStringLiteral("x"), QDateTime(dates[index], QTime(0, 0), Qt::UTC).toMSecsSinceEpoch()},
            {QStringLiteral("y"), values[index]}
        });
    }
    return points;
}

QVariantMap resultToVariant(const BacktestResult& result)
{
    return {
        {QStringLiteral("portfolioId"), result.portfolioId},
        {QStringLiteral("startDate"), result.startDate.toString(Qt::ISODate)},
        {QStringLiteral("endDate"), result.endDate.toString(Qt::ISODate)},
        {QStringLiteral("computedAt"), result.computedAt.toString(Qt::ISODate)},
        {QStringLiteral("metrics"), metricsToVariant(result.metrics)},
        {QStringLiteral("equityCurve"), toVariantList(result.equityCurve)},
        {QStringLiteral("drawdownCurve"), toVariantList(result.drawdownCurve)},
        {QStringLiteral("rollingYield"), toVariantList(result.rollingYield)},
        {QStringLiteral("dates"), toVariantList(result.dates)},
        {QStringLiteral("equityCurveData"), curveToPointList(result.dates, result.equityCurve)},
        {QStringLiteral("drawdownCurveData"), curveToPointList(result.dates, result.drawdownCurve)},
        {QStringLiteral("rollingYieldData"), curveToPointList(result.dates, result.rollingYield)}
    };
}

QJsonArray toJsonArray(const QVector<double>& values)
{
    QJsonArray array;
    for (double value : values) {
        array.append(value);
    }
    return array;
}

QJsonArray toJsonArray(const QVector<QDate>& values)
{
    QJsonArray array;
    for (const auto& value : values) {
        array.append(value.toString(Qt::ISODate));
    }
    return array;
}

QJsonObject resultToJson(const BacktestResult& result)
{
    return {
        {QStringLiteral("portfolioId"), result.portfolioId},
        {QStringLiteral("startDate"), result.startDate.toString(Qt::ISODate)},
        {QStringLiteral("endDate"), result.endDate.toString(Qt::ISODate)},
        {QStringLiteral("computedAt"), result.computedAt.toString(Qt::ISODate)},
        {QStringLiteral("metrics"), QJsonObject{
            {QStringLiteral("totalReturn"), result.metrics.totalReturn},
            {QStringLiteral("annualizedReturn"), result.metrics.annualizedReturn},
            {QStringLiteral("maxDrawdown"), result.metrics.maxDrawdown},
            {QStringLiteral("sharpeRatio"), result.metrics.sharpeRatio},
            {QStringLiteral("sortinoRatio"), result.metrics.sortinoRatio},
            {QStringLiteral("averageYield"), result.metrics.averageYield},
            {QStringLiteral("yieldStability"), result.metrics.yieldStability},
            {QStringLiteral("portfolioBeta"), result.metrics.portfolioBeta}
        }},
        {QStringLiteral("equityCurve"), toJsonArray(result.equityCurve)},
        {QStringLiteral("drawdownCurve"), toJsonArray(result.drawdownCurve)},
        {QStringLiteral("rollingYield"), toJsonArray(result.rollingYield)},
        {QStringLiteral("dates"), toJsonArray(result.dates)}
    };
}
}

BacktestController::BacktestController(QObject* parent)
    : QObject(parent)
    , m_optimizer(&m_dataStore)
    , m_backtestEngine(&m_dataStore)
{
    ensureSeedData();

    connect(&m_backtestWatcher, &QFutureWatcher<BacktestResult>::finished, this, [this]() {
        const BacktestResult backtestResult = m_backtestWatcher.result();
        setLoading(false);

        if (backtestResult.equityCurve.isEmpty()) {
            m_lastResult = {};
            m_hasLastResult = false;
            m_result.clear();
            emit resultChanged();
            setErrorMessage(QStringLiteral("Unable to generate a back-test for the selected inputs."));
            emit errorOccurred(m_errorMessage);
            return;
        }

        m_lastResult = backtestResult;
        m_hasLastResult = true;
        m_result = resultToVariant(backtestResult);
        emit resultChanged();

        const QString blobKey =
            QStringLiteral("backtest_%1_%2").arg(backtestResult.portfolioId, backtestResult.computedAt.toString(QStringLiteral("yyyyMMddhhmmss")));
        m_dataStore.saveBlob(blobKey, QJsonDocument(resultToJson(backtestResult)).toJson(QJsonDocument::Indented));

        setErrorMessage(QString());
        emit backtestCompleted();
    });
}

bool BacktestController::isLoading() const
{
    return m_isLoading;
}

QVariantMap BacktestController::result() const
{
    return m_result;
}

QString BacktestController::errorMessage() const
{
    return m_errorMessage;
}

bool BacktestController::hasLastResult() const
{
    return m_hasLastResult;
}

BacktestResult BacktestController::lastResult() const
{
    return m_lastResult;
}

int BacktestController::rebalanceFrequency() const
{
    return m_rebalanceFrequency;
}

void BacktestController::setRebalanceFrequency(int rebalanceFrequency)
{
    const int clamped = qBound(0, rebalanceFrequency, 3);
    if (m_rebalanceFrequency == clamped) {
        return;
    }

    m_rebalanceFrequency = clamped;
    emit rebalanceFrequencyChanged();
}

void BacktestController::runBacktest(double targetYield, int years)
{
    if (m_isLoading) {
        return;
    }

    const QVector<Holding> holdings = m_optimizer.optimize(targetYield, 5);
    if (holdings.isEmpty()) {
        m_lastResult = {};
        m_hasLastResult = false;
        m_result.clear();
        emit resultChanged();
        setErrorMessage(QStringLiteral("No qualifying assets available for the requested yield."));
        emit errorOccurred(m_errorMessage);
        return;
    }

    startBacktest(holdings, years);
}

void BacktestController::backtestPortfolio(QVariantList holdings, int years)
{
    if (m_isLoading) {
        return;
    }

    QVector<Holding> parsedHoldings;
    parsedHoldings.reserve(holdings.size());
    for (const auto& entry : holdings) {
        const QVariantMap map = entry.toMap();
        Holding holding;
        holding.ticker = map.value(QStringLiteral("ticker")).toString();
        holding.weight = map.value(QStringLiteral("weight")).toDouble();
        holding.yield = map.value(QStringLiteral("yield")).toDouble();
        holding.beta = map.value(QStringLiteral("beta")).toDouble();
        if (!holding.ticker.isEmpty()) {
            parsedHoldings.append(holding);
        }
    }

    if (parsedHoldings.isEmpty()) {
        m_lastResult = {};
        m_hasLastResult = false;
        m_result.clear();
        emit resultChanged();
        setErrorMessage(QStringLiteral("Please provide at least one holding to back-test."));
        emit errorOccurred(m_errorMessage);
        return;
    }

    startBacktest(parsedHoldings, years);
}

QVariantList BacktestController::equityCurveData() const
{
    return curveToPointList(m_lastResult.dates, m_lastResult.equityCurve);
}

QVariantList BacktestController::drawdownCurveData() const
{
    return curveToPointList(m_lastResult.dates, m_lastResult.drawdownCurve);
}

QVariantList BacktestController::rollingYieldData() const
{
    return curveToPointList(m_lastResult.dates, m_lastResult.rollingYield);
}

void BacktestController::ensureSeedData()
{
    if (!m_dataStore.loadAssets().isEmpty()) {
        return;
    }

    QVector<Asset> seedAssets = {
        {QStringLiteral("SCHD"), QStringLiteral("Schwab U.S. Dividend Equity ETF"), QStringLiteral("ETF"), 79.0, 0.036, 0.88, 50000000000.0, true, QDateTime::currentDateTimeUtc()},
        {QStringLiteral("VYM"), QStringLiteral("Vanguard High Dividend Yield ETF"), QStringLiteral("ETF"), 118.0, 0.031, 0.85, 54000000000.0, true, QDateTime::currentDateTimeUtc()},
        {QStringLiteral("KO"), QStringLiteral("Coca-Cola"), QStringLiteral("Consumer Defensive"), 63.0, 0.029, 0.58, 270000000000.0, false, QDateTime::currentDateTimeUtc()},
        {QStringLiteral("JNJ"), QStringLiteral("Johnson & Johnson"), QStringLiteral("Healthcare"), 148.0, 0.032, 0.54, 360000000000.0, false, QDateTime::currentDateTimeUtc()},
        {QStringLiteral("PG"), QStringLiteral("Procter & Gamble"), QStringLiteral("Consumer Defensive"), 165.0, 0.025, 0.43, 390000000000.0, false, QDateTime::currentDateTimeUtc()},
        {QStringLiteral("XLU"), QStringLiteral("Utilities Select Sector SPDR Fund"), QStringLiteral("Utilities"), 69.0, 0.034, 0.61, 13000000000.0, true, QDateTime::currentDateTimeUtc()}
    };

    m_dataStore.saveAssets(seedAssets);
}

void BacktestController::setErrorMessage(const QString& message)
{
    if (m_errorMessage == message) {
        return;
    }

    m_errorMessage = message;
    emit errorMessageChanged();
}

void BacktestController::setLoading(bool isLoading)
{
    if (m_isLoading == isLoading) {
        return;
    }

    m_isLoading = isLoading;
    emit isLoadingChanged();
}

void BacktestController::startBacktest(const QVector<Holding>& holdings, int years)
{
    if (years <= 0) {
        m_lastResult = {};
        m_hasLastResult = false;
        m_result.clear();
        emit resultChanged();
        setErrorMessage(QStringLiteral("Please choose a valid back-test duration."));
        emit errorOccurred(m_errorMessage);
        return;
    }

    setLoading(true);
    setErrorMessage(QString());

    const QVector<Holding> holdingsCopy = holdings;
    const QDate endDate = QDate::currentDate();
    const QDate startDate = endDate.addYears(-years);
    const RebalanceFrequency rebalance = static_cast<RebalanceFrequency>(m_rebalanceFrequency);

    m_backtestWatcher.setFuture(QtConcurrent::run([this, holdingsCopy, startDate, endDate, rebalance]() {
        return m_backtestEngine.run(holdingsCopy, startDate, endDate, rebalance);
    }));
}
