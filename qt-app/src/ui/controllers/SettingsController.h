#pragma once

#include <QFutureWatcher>
#include <QObject>
#include <QString>
#include <QVariantList>

class DataStore;
class UniverseManager;

class SettingsController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString apiKey READ apiKey WRITE setApiKey NOTIFY apiKeysChanged)
    Q_PROPERTY(QString alphaVantageApiKey READ alphaVantageApiKey WRITE setAlphaVantageApiKey NOTIFY apiKeysChanged)
    Q_PROPERTY(QString fmpApiKey READ fmpApiKey WRITE setFmpApiKey NOTIFY apiKeysChanged)
    Q_PROPERTY(QString polygonApiKey READ polygonApiKey WRITE setPolygonApiKey NOTIFY apiKeysChanged)
    Q_PROPERTY(QString fredApiKey READ fredApiKey WRITE setFredApiKey NOTIFY apiKeysChanged)
    Q_PROPERTY(bool hasApiKey READ hasApiKey NOTIFY apiKeysChanged)
    Q_PROPERTY(QString providerName READ providerName CONSTANT)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(int universeSize READ universeSize NOTIFY universeSizeChanged)
    Q_PROPERTY(bool isRefreshing READ isRefreshing NOTIFY isRefreshingChanged)
    Q_PROPERTY(int refreshProgress READ refreshProgress NOTIFY refreshProgressChanged)
    Q_PROPERTY(QVariantList universeTickers READ universeTickers NOTIFY universeSizeChanged)
    Q_PROPERTY(QString dataDirectory READ dataDirectory CONSTANT)
    Q_PROPERTY(bool isFirstRun READ isFirstRun NOTIFY firstRunChanged)

public:
    explicit SettingsController(UniverseManager* universeManager = nullptr, DataStore* dataStore = nullptr, QObject* parent = nullptr);
    ~SettingsController() override;

    QString apiKey() const;
    QString alphaVantageApiKey() const;
    QString fmpApiKey() const;
    QString polygonApiKey() const;
    QString fredApiKey() const;
    bool hasApiKey() const;
    void setApiKey(const QString& apiKey);
    void setAlphaVantageApiKey(const QString& apiKey);
    void setFmpApiKey(const QString& apiKey);
    void setPolygonApiKey(const QString& apiKey);
    void setFredApiKey(const QString& apiKey);

    QString providerName() const;
    QString statusMessage() const;
    int universeSize() const;
    bool isRefreshing() const;
    int refreshProgress() const;
    QVariantList universeTickers() const;
    QString dataDirectory() const;
    bool isFirstRun() const;

    Q_INVOKABLE void save();
    Q_INVOKABLE void load();
    Q_INVOKABLE void refreshUniverse();
    Q_INVOKABLE void addTicker(const QString& ticker);
    Q_INVOKABLE void removeTicker(const QString& ticker);
    Q_INVOKABLE void setFirstRunComplete();

signals:
    void apiKeysChanged();
    void statusMessageChanged();
    void universeSizeChanged();
    void isRefreshingChanged();
    void refreshProgressChanged();
    void firstRunChanged();

private:
    void notifyUniverseChanged();
    void setStatusMessage(const QString& message);
    void setIsRefreshing(bool isRefreshing);
    void setRefreshProgress(int refreshProgress);

    DataStore* m_dataStore;
    UniverseManager* m_universeManager;
    bool m_ownsDataStore = false;
    QString m_alphaVantageApiKey;
    QString m_fmpApiKey;
    QString m_polygonApiKey;
    QString m_fredApiKey;
    QString m_statusMessage;
    bool m_isFirstRun = false;
    bool m_isRefreshing = false;
    int m_refreshProgress = 0;
    QString m_pendingTicker;
    QFutureWatcher<int> m_refreshWatcher;
    QFutureWatcher<bool> m_tickerWatcher;
};
