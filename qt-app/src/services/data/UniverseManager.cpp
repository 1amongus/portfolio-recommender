#include "UniverseManager.h"

#include <algorithm>

#include <QDateTime>
#include <QSettings>
#include <QtConcurrent>

namespace {

Asset makeAsset(const QString& ticker,
                const QString& name,
                const QString& sector,
                double price,
                double dividendYield,
                double beta,
                double marketCap,
                bool isETF)
{
    return {
        ticker,
        name,
        sector,
        price,
        dividendYield,
        beta,
        marketCap,
        isETF,
        QDateTime::currentDateTimeUtc()
    };
}

QVector<Asset> defaultUniverse()
{
    return {
        makeAsset(QStringLiteral("SCHD"), QStringLiteral("Schwab U.S. Dividend Equity ETF"), QStringLiteral("ETF"), 79.0, 0.036, 0.88, 50000000000.0, true),
        makeAsset(QStringLiteral("VYM"), QStringLiteral("Vanguard High Dividend Yield ETF"), QStringLiteral("ETF"), 118.0, 0.031, 0.85, 54000000000.0, true),
        makeAsset(QStringLiteral("KO"), QStringLiteral("Coca-Cola"), QStringLiteral("Consumer Defensive"), 63.0, 0.029, 0.58, 270000000000.0, false),
        makeAsset(QStringLiteral("JNJ"), QStringLiteral("Johnson & Johnson"), QStringLiteral("Healthcare"), 148.0, 0.032, 0.54, 360000000000.0, false),
        makeAsset(QStringLiteral("PG"), QStringLiteral("Procter & Gamble"), QStringLiteral("Consumer Defensive"), 165.0, 0.025, 0.43, 390000000000.0, false),
        makeAsset(QStringLiteral("XLU"), QStringLiteral("Utilities Select Sector SPDR Fund"), QStringLiteral("Utilities"), 69.0, 0.034, 0.61, 13000000000.0, true),
        makeAsset(QStringLiteral("O"), QStringLiteral("Realty Income"), QStringLiteral("Real Estate"), 56.0, 0.054, 0.72, 49000000000.0, false),
        makeAsset(QStringLiteral("T"), QStringLiteral("AT&T"), QStringLiteral("Communication Services"), 18.0, 0.065, 0.75, 130000000000.0, false),
        makeAsset(QStringLiteral("MO"), QStringLiteral("Altria"), QStringLiteral("Consumer Defensive"), 45.0, 0.082, 0.63, 78000000000.0, false),
        makeAsset(QStringLiteral("ABBV"), QStringLiteral("AbbVie"), QStringLiteral("Healthcare"), 171.0, 0.038, 0.68, 300000000000.0, false),
        makeAsset(QStringLiteral("PFE"), QStringLiteral("Pfizer"), QStringLiteral("Healthcare"), 28.0, 0.057, 0.65, 160000000000.0, false),
        makeAsset(QStringLiteral("VZ"), QStringLiteral("Verizon"), QStringLiteral("Communication Services"), 40.0, 0.063, 0.42, 170000000000.0, false),
        makeAsset(QStringLiteral("IBM"), QStringLiteral("IBM"), QStringLiteral("Technology"), 173.0, 0.035, 0.95, 160000000000.0, false),
        makeAsset(QStringLiteral("CVX"), QStringLiteral("Chevron"), QStringLiteral("Energy"), 156.0, 0.039, 1.12, 290000000000.0, false),
        makeAsset(QStringLiteral("XOM"), QStringLiteral("ExxonMobil"), QStringLiteral("Energy"), 115.0, 0.033, 0.92, 460000000000.0, false),
        makeAsset(QStringLiteral("HDV"), QStringLiteral("iShares Core High Dividend ETF"), QStringLiteral("ETF"), 110.0, 0.038, 0.78, 11000000000.0, true),
        makeAsset(QStringLiteral("SPYD"), QStringLiteral("SPDR Portfolio S&P 500 High Dividend ETF"), QStringLiteral("ETF"), 43.0, 0.045, 0.91, 8000000000.0, true),
        makeAsset(QStringLiteral("DVY"), QStringLiteral("iShares Select Dividend ETF"), QStringLiteral("ETF"), 124.0, 0.036, 0.82, 19000000000.0, true),
        makeAsset(QStringLiteral("VIG"), QStringLiteral("Vanguard Dividend Appreciation ETF"), QStringLiteral("ETF"), 188.0, 0.019, 0.88, 82000000000.0, true),
        makeAsset(QStringLiteral("NOBL"), QStringLiteral("ProShares S&P 500 Dividend Aristocrats ETF"), QStringLiteral("ETF"), 98.0, 0.021, 0.85, 13000000000.0, true)
    };
}

QString normalizedTicker(const QString& ticker)
{
    return ticker.trimmed().toUpper();
}

int assetIndexForTicker(const QVector<Asset>& assets, const QString& ticker)
{
    const QString normalized = normalizedTicker(ticker);
    for (int index = 0; index < assets.size(); ++index) {
        if (assets.at(index).ticker.compare(normalized, Qt::CaseInsensitive) == 0) {
            return index;
        }
    }

    return -1;
}

void sortAssets(QVector<Asset>& assets)
{
    std::sort(assets.begin(), assets.end(), [](const Asset& left, const Asset& right) {
        return left.ticker < right.ticker;
    });
}

bool isValidFetchedAsset(const Asset& asset)
{
    return !asset.ticker.trimmed().isEmpty() && !asset.name.trimmed().isEmpty();
}

Asset fallbackAssetForTicker(const QString& ticker)
{
    const QString normalized = normalizedTicker(ticker);
    const auto seededAssets = defaultUniverse();
    const auto it = std::find_if(seededAssets.begin(), seededAssets.end(), [&](const Asset& asset) {
        return asset.ticker == normalized;
    });
    if (it != seededAssets.end()) {
        return *it;
    }

    return makeAsset(normalized,
                     normalized,
                     QStringLiteral("Unknown"),
                     100.0,
                     0.0,
                     1.0,
                     0.0,
                     false);
}

}

UniverseManager::UniverseManager(DataStore* dataStore, QObject* parent)
    : QObject(parent)
    , m_dataStore(dataStore)
{
}

QVector<Asset> UniverseManager::assets() const
{
    if (m_dataStore == nullptr) {
        return {};
    }

    auto currentAssets = m_dataStore->loadAssets();
    sortAssets(currentAssets);
    return currentAssets;
}

int UniverseManager::assetCount() const
{
    return assets().size();
}

bool UniverseManager::hasApiKey() const
{
    QSettings settings;
    return !settings.value(QStringLiteral("marketData/alphaVantageApiKey")).toString().trimmed().isEmpty();
}

QFuture<bool> UniverseManager::refreshTicker(const QString& ticker)
{
    return QtConcurrent::run([this, ticker]() {
        if (m_dataStore == nullptr) {
            emit errorOccurred(QStringLiteral("Asset storage is unavailable."));
            return false;
        }

        if (!hasApiKey()) {
            emit errorOccurred(QStringLiteral("Configure and save an Alpha Vantage API key before refreshing assets."));
            return false;
        }

        const QString normalized = normalizedTicker(ticker);
        if (normalized.isEmpty()) {
            emit errorOccurred(QStringLiteral("Please provide a valid ticker symbol."));
            return false;
        }

        const Asset fetchedAsset = m_provider.fetchAsset(normalized).result();
        if (!isValidFetchedAsset(fetchedAsset)) {
            emit errorOccurred(QStringLiteral("Unable to refresh %1 from Alpha Vantage.").arg(normalized));
            return false;
        }

        auto currentAssets = m_dataStore->loadAssets();
        const int existingIndex = assetIndexForTicker(currentAssets, normalized);
        if (existingIndex >= 0) {
            currentAssets[existingIndex] = fetchedAsset;
        } else {
            currentAssets.append(fetchedAsset);
        }

        sortAssets(currentAssets);
        if (!m_dataStore->saveAssets(currentAssets)) {
            emit errorOccurred(QStringLiteral("Failed to save the refreshed asset universe."));
            return false;
        }

        return true;
    });
}

QFuture<int> UniverseManager::refreshAll()
{
    return QtConcurrent::run([this]() {
        if (m_dataStore == nullptr) {
            emit errorOccurred(QStringLiteral("Asset storage is unavailable."));
            return 0;
        }

        if (!hasApiKey()) {
            emit errorOccurred(QStringLiteral("Configure and save an Alpha Vantage API key before refreshing assets."));
            return 0;
        }

        auto currentAssets = m_dataStore->loadAssets();
        const int total = currentAssets.size();
        if (total == 0) {
            emit refreshComplete(0);
            return 0;
        }

        int updated = 0;
        for (int index = 0; index < total; ++index) {
            const QString ticker = currentAssets.at(index).ticker;
            const Asset fetchedAsset = m_provider.fetchAsset(ticker).result();
            if (isValidFetchedAsset(fetchedAsset)) {
                currentAssets[index] = fetchedAsset;
                ++updated;
            } else {
                emit errorOccurred(QStringLiteral("Unable to refresh %1 from Alpha Vantage.").arg(ticker));
            }

            emit refreshProgress(index + 1, total);
        }

        if (updated > 0) {
            sortAssets(currentAssets);
            if (!m_dataStore->saveAssets(currentAssets)) {
                emit errorOccurred(QStringLiteral("Failed to save the refreshed asset universe."));
                return 0;
            }
        }

        emit refreshComplete(updated);
        return updated;
    });
}

QFuture<bool> UniverseManager::addTicker(const QString& ticker)
{
    return QtConcurrent::run([this, ticker]() {
        if (m_dataStore == nullptr) {
            emit errorOccurred(QStringLiteral("Asset storage is unavailable."));
            return false;
        }

        const QString normalized = normalizedTicker(ticker);
        if (normalized.isEmpty()) {
            emit errorOccurred(QStringLiteral("Please provide a valid ticker symbol."));
            return false;
        }

        auto currentAssets = m_dataStore->loadAssets();
        if (assetIndexForTicker(currentAssets, normalized) >= 0) {
            emit errorOccurred(QStringLiteral("%1 is already in the asset universe.").arg(normalized));
            return false;
        }

        Asset asset = hasApiKey() ? m_provider.fetchAsset(normalized).result() : fallbackAssetForTicker(normalized);
        if (hasApiKey() && !isValidFetchedAsset(asset)) {
            emit errorOccurred(QStringLiteral("Unable to add %1 from Alpha Vantage.").arg(normalized));
            return false;
        }

        if (!hasApiKey()) {
            asset = fallbackAssetForTicker(normalized);
        }

        currentAssets.append(asset);
        sortAssets(currentAssets);
        if (!m_dataStore->saveAssets(currentAssets)) {
            emit errorOccurred(QStringLiteral("Failed to save the updated asset universe."));
            return false;
        }

        return true;
    });
}

bool UniverseManager::removeTicker(const QString& ticker)
{
    if (m_dataStore == nullptr) {
        emit errorOccurred(QStringLiteral("Asset storage is unavailable."));
        return false;
    }

    auto currentAssets = m_dataStore->loadAssets();
    const int existingIndex = assetIndexForTicker(currentAssets, ticker);
    if (existingIndex < 0) {
        emit errorOccurred(QStringLiteral("Ticker %1 was not found in the asset universe.").arg(normalizedTicker(ticker)));
        return false;
    }

    currentAssets.removeAt(existingIndex);
    sortAssets(currentAssets);
    if (!m_dataStore->saveAssets(currentAssets)) {
        emit errorOccurred(QStringLiteral("Failed to save the updated asset universe."));
        return false;
    }

    return true;
}

void UniverseManager::ensureExpandedSeedData()
{
    if (m_dataStore == nullptr) {
        emit errorOccurred(QStringLiteral("Asset storage is unavailable."));
        return;
    }

    auto currentAssets = m_dataStore->loadAssets();
    if (currentAssets.size() > 6) {
        return;
    }

    const auto seededAssets = defaultUniverse();
    bool changed = false;
    for (const auto& asset : seededAssets) {
        if (assetIndexForTicker(currentAssets, asset.ticker) >= 0) {
            continue;
        }

        currentAssets.append(asset);
        changed = true;
    }

    if (!changed) {
        return;
    }

    sortAssets(currentAssets);
    if (!m_dataStore->saveAssets(currentAssets)) {
        emit errorOccurred(QStringLiteral("Failed to save the default asset universe."));
    }
}
