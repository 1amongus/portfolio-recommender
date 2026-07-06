#pragma once

#include <QObject>
#include <QString>
#include <QVector>

#include "../../models/BacktestResult.h"
#include "../../models/Portfolio.h"

class ExplanationEngine : public QObject
{
    Q_OBJECT

public:
    explicit ExplanationEngine(QObject* parent = nullptr);

    QString explainPortfolio(const QVector<Holding>& holdings,
                             double targetYield,
                             double achievedYield,
                             double portfolioBeta) const;
    QString explainBacktest(const BacktestMetrics& metrics, int years) const;
    QString explainSensitivity(double minBeta, double maxBeta, double optimalYield) const;
    QString explainRebalance(double originalBeta,
                             double newBeta,
                             double originalYield,
                             double newYield) const;

    bool isModelAvailable() const;

signals:
    void tokenGenerated(const QString& token);
    void generationComplete(const QString& fullText);
};
