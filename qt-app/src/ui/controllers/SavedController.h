#pragma once

#include <QObject>
#include <QVariantList>
#include <QVariantMap>

#include "../../services/data/DataStore.h"

class SavedController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList portfolios READ portfolios NOTIFY portfoliosChanged)
    Q_PROPERTY(QVariantMap selectedPortfolio READ selectedPortfolio NOTIFY selectedPortfolioChanged)
    Q_PROPERTY(int count READ count NOTIFY portfoliosChanged)

public:
    explicit SavedController(QObject* parent = nullptr);

    QVariantList portfolios() const;
    QVariantMap selectedPortfolio() const;
    int count() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void selectPortfolio(int index);
    Q_INVOKABLE void deletePortfolio(int index);
    Q_INVOKABLE void renamePortfolio(int index, const QString& newName);

signals:
    void portfoliosChanged();
    void selectedPortfolioChanged();

private:
    QVariantList m_portfolios;
    QVariantMap m_selectedPortfolio;
    DataStore m_dataStore;
    int m_selectedIndex = -1;
};
