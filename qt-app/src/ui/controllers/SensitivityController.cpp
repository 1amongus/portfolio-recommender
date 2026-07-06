#include "SensitivityController.h"



#include <QDateTime>

#include <QtConcurrent>

#include <QtMath>


#include "../../utils/Math.h"

namespace {
QVariantList holdingsToVariantList(const QVector<Holding>& holdings)
{
    QVariantList holdingsList;
    holdingsList.reserve(holdings.size());
    for (const auto& holding : holdings) {
        holdingsList.append(QVariantMap{
            {QStringLiteral("ticker"), holding.ticker},
            {QStringLiteral("weight"), holding.weight},
            {QStringLiteral("yield"), holding.yield},
            {QStringLiteral("beta"), holding.beta}
        });
    }
    return holdingsList;
}

QVariantMap pointToVariantMap(const SensitivityPoint& point)
{
    return {
        {QStringLiteral("yield"), point.yield},
        {QStringLiteral("beta"), point.beta},
        {QStringLiteral("achievedYield"), point.achievedYield},
        {QStringLiteral("holdings"), holdingsToVariantList(point.holdings)}
    };
}
}


SensitivityController::SensitivityController(QObject* parent)

    : QObject(parent)

    , m_minYield(0.01)

    , m_maxYield(0.08)

    , m_step(0.0025)

    , m_isLoading(false)

    , m_progress(0)

    , m_optimizer(&m_dataStore)

    , m_curveGenerator(&m_optimizer)

{

    ensureSeedData();



    connect(&m_curveGenerator, &CurveGenerator::progressChanged, this, [this](int current, int total) {

        if (total <= 0) {

            setProgress(0);

            return;

        }



        setProgress(qRound((static_cast<double>(current) / static_cast<double>(total)) * 100.0));

    });



    connect(&m_curveWatcher, &QFutureWatcher<SensitivityCurve>::finished, this, [this]() {

        const SensitivityCurve curve = m_curveWatcher.result();

        m_lastCurve = curve;

        updateCurveData(curve);

        setIsLoading(false);

        setProgress(100);



        if (curve.points.isEmpty()) {

            setErrorMessage(QStringLiteral("No portfolios could be generated in the requested yield range."));

            return;

        }



        if (!m_dataStore.saveSensitivityCurve(curve)) {

            setErrorMessage(QStringLiteral("Curve generated, but saving the results failed."));

            return;

        }



        setErrorMessage(QString());

    });

}



double SensitivityController::minYield() const

{

    return m_minYield;

}



void SensitivityController::setMinYield(double minYield)

{

    if (qFuzzyCompare(m_minYield + 1.0, minYield + 1.0)) {

        return;

    }



    m_minYield = minYield;

    emit minYieldChanged();

}



double SensitivityController::maxYield() const

{

    return m_maxYield;

}



void SensitivityController::setMaxYield(double maxYield)

{

    if (qFuzzyCompare(m_maxYield + 1.0, maxYield + 1.0)) {

        return;

    }



    m_maxYield = maxYield;

    emit maxYieldChanged();

}



double SensitivityController::step() const

{

    return m_step;

}



void SensitivityController::setStep(double step)

{

    if (qFuzzyCompare(m_step + 1.0, step + 1.0)) {

        return;

    }



    m_step = step;

    emit stepChanged();

}



bool SensitivityController::isLoading() const

{

    return m_isLoading;

}



int SensitivityController::progress() const

{

    return m_progress;

}



QVariantList SensitivityController::curveData() const

{

    return m_curveData;

}



QString SensitivityController::errorMessage() const

{

    return m_errorMessage;

}



double SensitivityController::currentPortfolioYield() const

{

    return m_currentPortfolioYield;

}



double SensitivityController::currentPortfolioBeta() const

{

    return m_currentPortfolioBeta;

}



void SensitivityController::generateCurve()

{

    if (m_isLoading) {

        return;

    }



    if (m_minYield < 0.0 || m_maxYield < 0.0) {

        setErrorMessage(QStringLiteral("Yield values must be non-negative."));

        return;

    }



    if (m_step <= 0.0) {

        setErrorMessage(QStringLiteral("Step size must be greater than zero."));

        return;

    }



    if (m_maxYield < m_minYield) {

        setErrorMessage(QStringLiteral("Maximum yield must be greater than or equal to minimum yield."));

        return;

    }



    setErrorMessage(QString());

    setProgress(0);

    setIsLoading(true);



    const double requestedMinYield = m_minYield;

    const double requestedMaxYield = m_maxYield;

    const double requestedStep = m_step;

    m_curveWatcher.setFuture(QtConcurrent::run([this, requestedMinYield, requestedMaxYield, requestedStep]() {

        return m_curveGenerator.generate(requestedMinYield, requestedMaxYield, requestedStep);

    }));

}



QVariantMap SensitivityController::pointDetails(int index) const

{

    if (index < 0 || index >= m_lastCurve.points.size()) {

        return {};

    }



    const auto& point = m_lastCurve.points[index];
    return pointToVariantMap(point);

}



void SensitivityController::setCurrentPortfolio(QVariantList holdings)

{

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



    const double newYield = portfolioYield(parsedHoldings);

    const double newBeta = portfolioBeta(parsedHoldings);

    if (qFuzzyCompare(m_currentPortfolioYield + 1.0, newYield + 1.0)
        && qFuzzyCompare(m_currentPortfolioBeta + 1.0, newBeta + 1.0)) {

        return;

    }



    m_currentPortfolioYield = newYield;

    m_currentPortfolioBeta = newBeta;

    emit currentPortfolioChanged();

}



QVariantMap SensitivityController::getPortfolioAtPoint(int index) const

{

    if (index < 0 || index >= m_lastCurve.points.size()) {

        return {};

    }



    const auto& point = m_lastCurve.points[index];

    return {

        {QStringLiteral("name"), QStringLiteral("Sensitivity Portfolio %1").arg(index + 1)},

        {QStringLiteral("targetYield"), point.yield},

        {QStringLiteral("achievedYield"), point.achievedYield},

        {QStringLiteral("aggregateBeta"), point.beta},

        {QStringLiteral("holdings"), holdingsToVariantList(point.holdings)}

    };

}



void SensitivityController::ensureSeedData()

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



void SensitivityController::setIsLoading(bool isLoading)

{

    if (m_isLoading == isLoading) {

        return;

    }



    m_isLoading = isLoading;

    emit isLoadingChanged();

}



void SensitivityController::setProgress(int progress)

{

    progress = qBound(0, progress, 100);

    if (m_progress == progress) {

        return;

    }



    m_progress = progress;

    emit progressChanged();

}



void SensitivityController::setErrorMessage(const QString& message)

{

    if (m_errorMessage == message) {

        return;

    }



    m_errorMessage = message;

    emit errorMessageChanged();

}



void SensitivityController::updateCurveData(const SensitivityCurve& curve)

{

    QVariantList points;

    points.reserve(curve.points.size());



    for (const auto& point : curve.points) {

        points.append(QVariantMap {

            {QStringLiteral("yield"), point.yield},

            {QStringLiteral("beta"), point.beta},

            {QStringLiteral("achievedYield"), point.achievedYield}

        });

    }



    m_curveData = points;

    emit curveDataChanged();

}
