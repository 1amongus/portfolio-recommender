#include "SettingsController.h"

#include <QSettings>

#include "../../services/data/DataStore.h"
#include "../../services/data/UniverseManager.h"

namespace {
const QString kAlphaVantageApiKeySetting = QStringLiteral("marketData/alphaVantageApiKey");
const QString kFmpApiKeySetting = QStringLiteral("marketData/fmpApiKey");
const QString kPolygonApiKeySetting = QStringLiteral("marketData/polygonApiKey");
const QString kFredApiKeySetting = QStringLiteral("marketData/fredApiKey");
const QString kFirstRunSetting = QStringLiteral("app/firstRunComplete");
}

SettingsController::SettingsController(UniverseManager* universeManager, DataStore* dataStore, QObject* parent)
    : QObject(parent)
    , m_dataStore(dataStore != nullptr ? dataStore : new DataStore())
    , m_universeManager(universeManager != nullptr ? universeManager : new UniverseManager(m_dataStore, this))
    , m_ownsDataStore(dataStore == nullptr)
{
    connect(m_universeManager, &UniverseManager::refreshProgress, this, [this](int current, int total) {
        if (total <= 0) {
            setRefreshProgress(0);
            return;
        }

        setRefreshProgress(static_cast<int>((current * 100.0) / total));
        setStatusMessage(QStringLiteral("Refreshing universe... %1 of %2").arg(current).arg(total));
    });

    connect(m_universeManager, &UniverseManager::refreshComplete, this, [this](int updated) {
        setRefreshProgress(100);
        notifyUniverseChanged();
        setStatusMessage(QStringLiteral("Universe refresh complete. Updated %1 tickers.").arg(updated));
    });

    connect(m_universeManager, &UniverseManager::errorOccurred, this, [this](const QString& message) {
        setStatusMessage(message);
    });

    connect(&m_refreshWatcher, &QFutureWatcher<int>::finished, this, [this]() {
        setIsRefreshing(false);
        if (!m_refreshWatcher.isCanceled() && m_refreshProgress == 0 && hasApiKey()) {
            setRefreshProgress(100);
        }
    });

    connect(&m_tickerWatcher, &QFutureWatcher<bool>::finished, this, [this]() {
        setIsRefreshing(false);
        if (m_tickerWatcher.result()) {
            notifyUniverseChanged();
            setStatusMessage(QStringLiteral("Added %1 to the asset universe.").arg(m_pendingTicker));
        }

        m_pendingTicker.clear();
    });

    load();
    m_universeManager->ensureExpandedSeedData();
    notifyUniverseChanged();
}

SettingsController::~SettingsController()
{
    if (m_ownsDataStore) {
        delete m_dataStore;
    }
}

QString SettingsController::apiKey() const
{
    return m_alphaVantageApiKey;
}

QString SettingsController::alphaVantageApiKey() const
{
    return m_alphaVantageApiKey;
}

QString SettingsController::fmpApiKey() const
{
    return m_fmpApiKey;
}

QString SettingsController::polygonApiKey() const
{
    return m_polygonApiKey;
}

QString SettingsController::fredApiKey() const
{
    return m_fredApiKey;
}

bool SettingsController::hasApiKey() const
{
    return !m_alphaVantageApiKey.trimmed().isEmpty()
        || !m_fmpApiKey.trimmed().isEmpty()
        || !m_polygonApiKey.trimmed().isEmpty()
        || !m_fredApiKey.trimmed().isEmpty();
}

void SettingsController::setApiKey(const QString& apiKey)
{
    setAlphaVantageApiKey(apiKey);
}

void SettingsController::setAlphaVantageApiKey(const QString& apiKey)
{
    if (m_alphaVantageApiKey == apiKey) {
        return;
    }

    m_alphaVantageApiKey = apiKey;
    emit apiKeysChanged();
}

void SettingsController::setFmpApiKey(const QString& apiKey)
{
    if (m_fmpApiKey == apiKey) {
        return;
    }

    m_fmpApiKey = apiKey;
    emit apiKeysChanged();
}

void SettingsController::setPolygonApiKey(const QString& apiKey)
{
    if (m_polygonApiKey == apiKey) {
        return;
    }

    m_polygonApiKey = apiKey;
    emit apiKeysChanged();
}

void SettingsController::setFredApiKey(const QString& apiKey)
{
    if (m_fredApiKey == apiKey) {
        return;
    }

    m_fredApiKey = apiKey;
    emit apiKeysChanged();
}

QString SettingsController::providerName() const
{
    return QStringLiteral("Alpha Vantage / FMP / Polygon / FRED");
}

QString SettingsController::statusMessage() const
{
    return m_statusMessage;
}

int SettingsController::universeSize() const
{
    return m_universeManager != nullptr ? m_universeManager->assetCount() : 0;
}

bool SettingsController::isRefreshing() const
{
    return m_isRefreshing;
}

int SettingsController::refreshProgress() const
{
    return m_refreshProgress;
}

QVariantList SettingsController::universeTickers() const
{
    QVariantList tickers;
    if (m_universeManager == nullptr) {
        return tickers;
    }

    const auto assets = m_universeManager->assets();
    tickers.reserve(assets.size());
    for (const auto& asset : assets) {
        QVariantMap item;
        item.insert(QStringLiteral("ticker"), asset.ticker);
        item.insert(QStringLiteral("name"), asset.name);
        item.insert(QStringLiteral("sector"), asset.sector);
        item.insert(QStringLiteral("yield"), asset.dividendYield);
        item.insert(QStringLiteral("beta"), asset.beta);
        item.insert(QStringLiteral("lastUpdated"), asset.lastUpdated.toLocalTime().toString(QStringLiteral("yyyy-MM-dd hh:mm")));
        tickers.append(item);
    }

    return tickers;
}

QString SettingsController::dataDirectory() const
{
    return m_dataStore != nullptr ? m_dataStore->dataDirectory() : QString();
}

bool SettingsController::isFirstRun() const
{
    return m_isFirstRun;
}

void SettingsController::save()
{
    QSettings settings;
    settings.setValue(kAlphaVantageApiKeySetting, m_alphaVantageApiKey);
    settings.setValue(kFmpApiKeySetting, m_fmpApiKey);
    settings.setValue(kPolygonApiKeySetting, m_polygonApiKey);
    settings.setValue(kFredApiKeySetting, m_fredApiKey);
    emit apiKeysChanged();
    setStatusMessage(QStringLiteral("Settings saved."));
}

void SettingsController::load()
{
    QSettings settings;
    setAlphaVantageApiKey(settings.value(kAlphaVantageApiKeySetting).toString());
    setFmpApiKey(settings.value(kFmpApiKeySetting).toString());
    setPolygonApiKey(settings.value(kPolygonApiKeySetting).toString());
    setFredApiKey(settings.value(kFredApiKeySetting).toString());

    const bool firstRunComplete = settings.value(kFirstRunSetting, false).toBool();
    const bool isFirstRun = !firstRunComplete && !hasApiKey();
    if (m_isFirstRun != isFirstRun) {
        m_isFirstRun = isFirstRun;
        emit firstRunChanged();
    }

    emit apiKeysChanged();
    setStatusMessage(QStringLiteral("Settings loaded."));
}

void SettingsController::refreshUniverse()
{
    if (m_universeManager == nullptr) {
        setStatusMessage(QStringLiteral("Universe management is unavailable."));
        return;
    }

    if (m_isRefreshing) {
        setStatusMessage(QStringLiteral("A universe update is already in progress."));
        return;
    }

    if (!hasApiKey()) {
        setStatusMessage(QStringLiteral("Save at least one market data API key before refreshing the universe."));
        return;
    }

    setIsRefreshing(true);
    setRefreshProgress(0);
    setStatusMessage(QStringLiteral("Refreshing universe..."));
    m_refreshWatcher.setFuture(m_universeManager->refreshAll());
}

void SettingsController::addTicker(const QString& ticker)
{
    if (m_universeManager == nullptr) {
        setStatusMessage(QStringLiteral("Universe management is unavailable."));
        return;
    }

    if (m_isRefreshing) {
        setStatusMessage(QStringLiteral("Please wait for the current universe update to finish."));
        return;
    }

    m_pendingTicker = ticker.trimmed().toUpper();
    if (m_pendingTicker.isEmpty()) {
        setStatusMessage(QStringLiteral("Please provide a ticker to add."));
        return;
    }

    setIsRefreshing(true);
    setRefreshProgress(0);
    setStatusMessage(QStringLiteral("Adding %1...").arg(m_pendingTicker));
    m_tickerWatcher.setFuture(m_universeManager->addTicker(m_pendingTicker));
}

void SettingsController::removeTicker(const QString& ticker)
{
    if (m_universeManager == nullptr) {
        setStatusMessage(QStringLiteral("Universe management is unavailable."));
        return;
    }

    if (m_isRefreshing) {
        setStatusMessage(QStringLiteral("Please wait for the current universe update to finish."));
        return;
    }

    const QString normalized = ticker.trimmed().toUpper();
    if (normalized.isEmpty()) {
        setStatusMessage(QStringLiteral("Please provide a ticker to remove."));
        return;
    }

    if (m_universeManager->removeTicker(normalized)) {
        notifyUniverseChanged();
        setStatusMessage(QStringLiteral("Removed %1 from the asset universe.").arg(normalized));
    }
}

void SettingsController::setFirstRunComplete()
{
    QSettings settings;
    settings.setValue(kFirstRunSetting, true);
    if (!m_isFirstRun) {
        return;
    }

    m_isFirstRun = false;
    emit firstRunChanged();
}

void SettingsController::notifyUniverseChanged()
{
    emit universeSizeChanged();
}

void SettingsController::setStatusMessage(const QString& message)
{
    if (m_statusMessage == message) {
        return;
    }

    m_statusMessage = message;
    emit statusMessageChanged();
}

void SettingsController::setIsRefreshing(bool isRefreshing)
{
    if (m_isRefreshing == isRefreshing) {
        return;
    }

    m_isRefreshing = isRefreshing;
    emit isRefreshingChanged();
}

void SettingsController::setRefreshProgress(int refreshProgress)
{
    refreshProgress = qBound(0, refreshProgress, 100);
    if (m_refreshProgress == refreshProgress) {
        return;
    }

    m_refreshProgress = refreshProgress;
    emit refreshProgressChanged();
}
