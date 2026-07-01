#pragma once

#include <QVector>

#include "../../models/Portfolio.h"

class DataStore;

class Optimizer
{
public:
    explicit Optimizer(DataStore* dataStore = nullptr);

    QVector<Holding> optimize(double targetYield, int maxAssets = 5) const;

private:
    DataStore* m_dataStore;
};