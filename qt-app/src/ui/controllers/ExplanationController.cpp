#include "ExplanationController.h"

#include <QRegularExpression>
#include <QStringList>
#include <QTimer>

#include <functional>
#include <memory>

ExplanationController::ExplanationController(QObject* parent)
    : QObject(parent)
{
}

QString ExplanationController::explanation() const
{
    return m_explanation;
}

bool ExplanationController::isGenerating() const
{
    return m_isGenerating;
}

bool ExplanationController::modelAvailable() const
{
    return m_engine.isModelAvailable();
}

void ExplanationController::explainPortfolio(QVariantList holdings,
                                             double targetYield,
                                             double achievedYield,
                                             double beta)
{
    streamText(m_engine.explainPortfolio(holdingsFromVariantList(holdings), targetYield, achievedYield, beta));
}

void ExplanationController::explainBacktest(QVariantMap metrics, int years)
{
    streamText(m_engine.explainBacktest(metricsFromVariantMap(metrics), years));
}

void ExplanationController::explainSensitivity(double minBeta, double maxBeta, double optimalYield)
{
    streamText(m_engine.explainSensitivity(minBeta, maxBeta, optimalYield));
}

void ExplanationController::explainRebalance(double origBeta, double newBeta, double origYield, double newYield)
{
    streamText(m_engine.explainRebalance(origBeta, newBeta, origYield, newYield));
}

void ExplanationController::clear()
{
    ++m_generationId;
    setExplanation(QString());
    setIsGenerating(false);
}

QVector<Holding> ExplanationController::holdingsFromVariantList(const QVariantList& holdings) const
{
    QVector<Holding> parsedHoldings;
    parsedHoldings.reserve(holdings.size());

    for (const auto& entry : holdings) {
        const QVariantMap map = entry.toMap();
        const QString ticker = map.value(QStringLiteral("ticker")).toString().trimmed();
        if (ticker.isEmpty()) {
            continue;
        }

        Holding holding;
        holding.ticker = ticker;
        holding.weight = map.value(QStringLiteral("weight")).toDouble();
        holding.yield = map.value(QStringLiteral("yield")).toDouble();
        holding.beta = map.value(QStringLiteral("beta")).toDouble();
        parsedHoldings.append(holding);
    }

    return parsedHoldings;
}

BacktestMetrics ExplanationController::metricsFromVariantMap(const QVariantMap& metrics) const
{
    BacktestMetrics parsedMetrics;
    parsedMetrics.totalReturn = metrics.value(QStringLiteral("totalReturn")).toDouble();
    parsedMetrics.annualizedReturn = metrics.value(QStringLiteral("annualizedReturn")).toDouble();
    parsedMetrics.maxDrawdown = metrics.value(QStringLiteral("maxDrawdown")).toDouble();
    parsedMetrics.sharpeRatio = metrics.value(QStringLiteral("sharpeRatio")).toDouble();
    parsedMetrics.sortinoRatio = metrics.value(QStringLiteral("sortinoRatio")).toDouble();
    parsedMetrics.averageYield = metrics.value(QStringLiteral("averageYield")).toDouble();
    parsedMetrics.yieldStability = metrics.value(QStringLiteral("yieldStability")).toDouble();
    parsedMetrics.portfolioBeta = metrics.value(QStringLiteral("portfolioBeta")).toDouble();
    return parsedMetrics;
}

void ExplanationController::setExplanation(const QString& explanation)
{
    if (m_explanation == explanation) {
        return;
    }

    m_explanation = explanation;
    emit explanationChanged();
}

void ExplanationController::setIsGenerating(bool isGenerating)
{
    if (m_isGenerating == isGenerating) {
        return;
    }

    m_isGenerating = isGenerating;
    emit isGeneratingChanged();
}

void ExplanationController::streamText(const QString& text)
{
    const int generationId = ++m_generationId;
    setExplanation(QString());

    if (text.trimmed().isEmpty()) {
        setIsGenerating(false);
        return;
    }

    setIsGenerating(true);

    const QStringList words = text.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    auto currentIndex = std::make_shared<int>(0);
    auto streamedWords = std::make_shared<QStringList>();
    auto pump = std::make_shared<std::function<void()>>();

    *pump = [this, text, words, currentIndex, streamedWords, generationId, pump]() {
        if (generationId != m_generationId) {
            return;
        }

        if (*currentIndex >= words.size()) {
            setExplanation(text);
            setIsGenerating(false);
            return;
        }

        streamedWords->append(words.at(*currentIndex));
        ++(*currentIndex);
        setExplanation(streamedWords->join(QLatin1Char(' ')));

        QTimer::singleShot(20, this, [generationId, pump, this]() {
            if (generationId == m_generationId) {
                (*pump)();
            }
        });
    };

    (*pump)();
}
