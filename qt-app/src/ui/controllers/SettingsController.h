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
    Q_PROPERTY(QString apiKey READ apiKey WRITE setApiKey NOTIFY apiKeyChanged)
    Q_PROPERTY(bool hasApiKey READ hasApiKey NOTIFY apiKeyChanged)
    Q_PROPERTY(QString providerName READ providerName CONSTANT)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(int universeSize READ universeSize NOTIFY universeSizeChanged)
    Q_PROPERTY(bool isRefreshing READ isRefreshing NOTIFY isRefreshingChanged)
    Q_PROPERTY(int refreshProgress READ refreshProgress NOTIFY refreshProgressChanged)
    Q_PROPERTY(QVariantList universeTickers READ universeTickers NOTIFY universeSizeChanged)
    Q_PROPERTY(QString dataDirectory READ dataDirectory CONSTANT)

public:
    explicit SettingsController(UniverseManager* universeManager = nullptr, DataStore* dataStore = nullptr, QObject* parent = nullptr);
    ~SettingsController() override;

    QString apiKey() const;
    bool hasApiKey() const;
    void setApiKey(const QString& apiKey);

    QString providerName() const;
    QString statusMessage() const;
    int universeSize() const;
    bool isRefreshing() const;
    int refreshProgress() const;
    QVariantList universeTickers() const;
    QString dataDirectory() const;

    Q_INVOKABLE void save();
    Q_INVOKABLE void load();
    Q_INVOKABLE void refreshUniverse();
    Q_INVOKABLE void addTicker(const QString& ticker);
    Q_INVOKABLE void removeTicker(const QString& ticker);

signals:
    void apiKeyChanged();
    void statusMessageChanged();
    void universeSizeChanged();
    void isRefreshingChanged();
    void refreshProgressChanged();

private:
    void notifyUniverseChanged();
    void setStatusMessage(const QString& message);
    void setIsRefreshing(bool isRefreshing);
    void setRefreshProgress(int refreshProgress);

    DataStore* m_dataStore;
    UniverseManager* m_universeManager;
    bool m_ownsDataStore = false;
    QString m_apiKey;
    QString m_statusMessage;
    bool m_isRefreshing = false;
    int m_refreshProgress = 0;
    QString m_pendingTicker;
    QFutureWatcher<int> m_refreshWatcher;
    QFutureWatcher<bool> m_tickerWatcher;
};