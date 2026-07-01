#include "Optimizer.h"

#include <algorithm>

#include "../../models/Asset.h"
#include "../../utils/Math.h"
#include "../data/DataStore.h"

Optimizer::Optimizer(DataStore* dataStore)
    : m_dataStore(dataStore)
{
}

QVector<Holding> Optimizer::optimize(double targetYield, int maxAssets) const
{
    if (m_dataStore == nullptr || maxAssets <= 0) {
        return {};
    }

    QVector<Asset> universe = m_dataStore->loadAssets();
    universe.erase(std::remove_if(universe.begin(), universe.end(), [](const Asset& asset) {
        return asset.dividendYield <= 0.0;
    }), universe.end());

    std::sort(universe.begin(), universe.end(), [](const Asset& left, const Asset& right) {
        if (!qFuzzyCompare(left.beta + 1.0, right.beta + 1.0)) {
            return left.beta < right.beta;
        }
        return left.dividendYield > right.dividendYield;
    });

    if (universe.isEmpty()) {
        return {};
    }

    QVector<Holding> holdings;
    QVector<Asset> qualifying;
    for (const auto& asset : universe) {
        if (asset.dividendYield + 1e-6 >= targetYield) {
            qualifying.append(asset);
        }
    }

    const QVector<Asset>& selection = qualifying.isEmpty() ? universe : qualifying;
    const int selectionCount = qMin(maxAssets, selection.size());
    for (int i = 0; i < selectionCount; ++i) {
        holdings.append({selection[i].ticker, 0.0, selection[i].dividendYield, selection[i].beta});
    }

    if (holdings.isEmpty()) {
        return {};
    }

    if (!qualifying.isEmpty()) {
        holdings[0].weight = 1.0;
        holdings.resize(1);
        return holdings;
    }

    const double equalWeight = 1.0 / static_cast<double>(holdings.size());
    for (auto& holding : holdings) {
        holding.weight = equalWeight;
    }

    double currentYield = portfolioYield(holdings);
    for (int iteration = 0; iteration < 250 && currentYield + 1e-6 < targetYield; ++iteration) {
        int lowYieldIndex = 0;
        int highYieldIndex = 0;
        for (int i = 1; i < holdings.size(); ++i) {
            if (holdings[i].yield < holdings[lowYieldIndex].yield ||
                (qFuzzyCompare(holdings[i].yield + 1.0, holdings[lowYieldIndex].yield + 1.0) && holdings[i].beta > holdings[lowYieldIndex].beta)) {
                lowYieldIndex = i;
            }
            if (holdings[i].yield > holdings[highYieldIndex].yield ||
                (qFuzzyCompare(holdings[i].yield + 1.0, holdings[highYieldIndex].yield + 1.0) && holdings[i].beta < holdings[highYieldIndex].beta)) {
                highYieldIndex = i;
            }
        }

        if (lowYieldIndex == highYieldIndex) {
            break;
        }

        const double shift = qMin(0.02, holdings[lowYieldIndex].weight);
        if (shift <= 0.0) {
            break;
        }

        holdings[lowYieldIndex].weight -= shift;
        holdings[highYieldIndex].weight += shift;
        currentYield = portfolioYield(holdings);
    }

    double totalWeight = 0.0;
    for (const auto& holding : holdings) {
        totalWeight += holding.weight;
    }

    if (totalWeight > 0.0) {
        for (auto& holding : holdings) {
            holding.weight /= totalWeight;
        }
    }

    holdings.erase(std::remove_if(holdings.begin(), holdings.end(), [](const Holding& holding) {
        return holding.weight <= 1e-6;
    }), holdings.end());

    std::sort(holdings.begin(), holdings.end(), [](const Holding& left, const Holding& right) {
        return left.weight > right.weight;
    });

    return holdings;
}