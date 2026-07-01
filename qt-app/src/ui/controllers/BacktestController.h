#pragma once

#include <QFutureWatcher>
#include <QObject>
#include <QVariantMap>

#include "../../models/BacktestResult.h"
#include "../../models/Portfolio.h"
#include "../../services/backtest/Engine.h"
#include "../../services/data/DataStore.h"
#include "../../services/optimization/Optimizer.h"

class BacktestController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(QVariantMap result READ result NOTIFY resultChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

public:
    explicit BacktestController(QObject* parent = nullptr);

    bool isLoading() const;
    QVariantMap result() const;
    QString errorMessage() const;

    Q_INVOKABLE void runBacktest(double targetYield, int years);
    Q_INVOKABLE void backtestPortfolio(QVariantList holdings, int years);

signals:
    void isLoadingChanged();
    void resultChanged();
    void errorMessageChanged();
    void backtestCompleted();
    void errorOccurred(const QString& message);

private:
    void ensureSeedData();
    void setErrorMessage(const QString& message);
    void setLoading(bool isLoading);
    void startBacktest(const QVector<Holding>& holdings, int years);

    bool m_isLoading = false;
    QVariantMap m_result;
    QString m_errorMessage;
    DataStore m_dataStore;
    Optimizer m_optimizer;
    BacktestEngine m_backtestEngine;
    QFutureWatcher<BacktestResult> m_backtestWatcher;
};
