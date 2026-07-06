#pragma once

#include <QObject>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>

#include "../../models/Asset.h"
#include "../../models/BacktestResult.h"
#include "../../models/Portfolio.h"
#include "../../services/backtest/Engine.h"
#include "../../services/data/DataStore.h"
#include "../../services/import/CsvParser.h"
#include "../../services/optimization/Optimizer.h"

class ImportController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList importedHoldings READ importedHoldings NOTIFY importedHoldingsChanged)
    Q_PROPERTY(QVariantList rebalancedHoldings READ rebalancedHoldings NOTIFY rebalancedHoldingsChanged)
    Q_PROPERTY(QVariantMap comparison READ comparison NOTIFY comparisonChanged)
    Q_PROPERTY(QVariantMap originalBacktest READ originalBacktest NOTIFY originalBacktestChanged)
    Q_PROPERTY(QVariantMap rebalancedBacktest READ rebalancedBacktest NOTIFY rebalancedBacktestChanged)
    Q_PROPERTY(double targetYield READ targetYield WRITE setTargetYield NOTIFY targetYieldChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(QStringList parseWarnings READ parseWarnings NOTIFY parseWarningsChanged)

public:
    explicit ImportController(QObject* parent = nullptr);

    QVariantList importedHoldings() const;
    QVariantList rebalancedHoldings() const;
    QVariantMap comparison() const;
    QVariantMap originalBacktest() const;
    QVariantMap rebalancedBacktest() const;
    double targetYield() const;
    bool isLoading() const;
    QString errorMessage() const;
    QStringList parseWarnings() const;

    void setTargetYield(double targetYield);

    Q_INVOKABLE void importFromFile(const QString& filePath);
    Q_INVOKABLE void importFromText(const QString& csvText);
    Q_INVOKABLE void rebalance();
    Q_INVOKABLE void addHolding(const QString& ticker, double weight);
    Q_INVOKABLE void removeHolding(int index);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void runComparativeBacktest(int years);

signals:
    void importedHoldingsChanged();
    void rebalancedHoldingsChanged();
    void comparisonChanged();
    void originalBacktestChanged();
    void rebalancedBacktestChanged();
    void targetYieldChanged();
    void isLoadingChanged();
    void errorMessageChanged();
    void parseWarningsChanged();

private:
    QVariantList holdingsToVariantList(const QVector<Holding>& holdings) const;
    QVariantMap buildComparison(const QVector<Holding>& original, const QVector<Holding>& rebalanced) const;
    QVector<Holding> normalizedHoldings(const QVector<Holding>& holdings) const;
    Holding buildHolding(const QString& ticker, double weight, bool* foundInUniverse = nullptr) const;
    void applyParseResult(const ParseResult& result);
    void setImportedHoldingsData(const QVector<Holding>& holdings);
    void setRebalancedHoldingsData(const QVector<Holding>& holdings);
    void setComparisonData(const QVariantMap& comparison);
    void setOriginalBacktestData(const QVariantMap& backtest);
    void setRebalancedBacktestData(const QVariantMap& backtest);
    void setIsLoading(bool isLoading);
    void setErrorMessage(const QString& message);
    void setParseWarnings(const QStringList& warnings);

    QVector<Holding> m_importedHoldingsData;
    QVector<Holding> m_rebalancedHoldingsData;
    QVariantList m_importedHoldings;
    QVariantList m_rebalancedHoldings;
    QVariantMap m_comparison;
    QVariantMap m_originalBacktest;
    QVariantMap m_rebalancedBacktest;
    double m_targetYield;
    bool m_isLoading;
    QString m_errorMessage;
    QStringList m_parseWarnings;
    DataStore m_dataStore;
    Optimizer m_optimizer;
    BacktestEngine m_backtestEngine;
    CsvParser m_parser;
};
