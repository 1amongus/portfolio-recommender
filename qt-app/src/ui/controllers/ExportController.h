#pragma once

#include <QObject>
#include <QString>

#include "../../services/data/DataStore.h"

class BacktestController;

class ExportController : public QObject
{
    Q_OBJECT

public:
    explicit ExportController(QObject* parent = nullptr);

    void setBacktestController(BacktestController* backtestController);

    Q_INVOKABLE void exportPortfolioCsv(int portfolioIndex);
    Q_INVOKABLE void exportPortfolioPdf(int portfolioIndex);
    Q_INVOKABLE void exportBacktestCsv();
    Q_INVOKABLE QString getExportPath(const QString& defaultName, const QString& filter);

private:
    QString defaultExportDirectory() const;
    QString safeDefaultName(const QString& defaultName) const;

    DataStore m_dataStore;
    BacktestController* m_backtestController = nullptr;
};
