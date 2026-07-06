#pragma once

#include <QObject>
#include <QVariantMap>

#include "../../models/Portfolio.h"
#include "../../services/data/DataStore.h"
#include "../../services/data/UniverseManager.h"
#include "../../services/optimization/Optimizer.h"

class PortfolioController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double targetYield READ targetYield WRITE setTargetYield NOTIFY targetYieldChanged)
    Q_PROPERTY(QVariantMap portfolioResult READ portfolioResult NOTIFY portfolioResultChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

public:
    explicit PortfolioController(QObject* parent = nullptr);

    double targetYield() const;
    void setTargetYield(double targetYield);

    QVariantMap portfolioResult() const;
    bool isLoading() const;
    QString errorMessage() const;

    Q_INVOKABLE void generate();
    Q_INVOKABLE void loadPortfolio(const QVariantMap& portfolioData);

signals:
    void targetYieldChanged();
    void portfolioResultChanged();
    void isLoadingChanged();
    void errorMessageChanged();
    void portfolioReady();
    void errorOccurred(const QString& message);

private:
    void setErrorMessage(const QString& message);

    double m_targetYield;
    bool m_isLoading;
    QString m_errorMessage;
    QVariantMap m_portfolioResult;
    DataStore m_dataStore;
    UniverseManager m_universeManager;
    Optimizer m_optimizer;
};