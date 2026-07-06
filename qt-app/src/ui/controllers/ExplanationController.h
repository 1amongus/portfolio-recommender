#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

#include "../../services/slm/ExplanationEngine.h"

class ExplanationController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString explanation READ explanation NOTIFY explanationChanged)
    Q_PROPERTY(bool isGenerating READ isGenerating NOTIFY isGeneratingChanged)
    Q_PROPERTY(bool modelAvailable READ modelAvailable CONSTANT)

public:
    explicit ExplanationController(QObject* parent = nullptr);

    QString explanation() const;
    bool isGenerating() const;
    bool modelAvailable() const;

    Q_INVOKABLE void explainPortfolio(QVariantList holdings, double targetYield, double achievedYield, double beta);
    Q_INVOKABLE void explainBacktest(QVariantMap metrics, int years);
    Q_INVOKABLE void explainSensitivity(double minBeta, double maxBeta, double optimalYield);
    Q_INVOKABLE void explainRebalance(double origBeta, double newBeta, double origYield, double newYield);
    Q_INVOKABLE void clear();

signals:
    void explanationChanged();
    void isGeneratingChanged();

private:
    QVector<Holding> holdingsFromVariantList(const QVariantList& holdings) const;
    BacktestMetrics metricsFromVariantMap(const QVariantMap& metrics) const;
    void setExplanation(const QString& explanation);
    void setIsGenerating(bool isGenerating);
    void streamText(const QString& text);

    ExplanationEngine m_engine;
    QString m_explanation;
    bool m_isGenerating = false;
    int m_generationId = 0;
};
