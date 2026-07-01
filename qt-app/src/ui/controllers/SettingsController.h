#pragma once

#include <QObject>
#include <QString>

class SettingsController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString apiKey READ apiKey WRITE setApiKey NOTIFY apiKeyChanged)
    Q_PROPERTY(QString providerName READ providerName CONSTANT)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

public:
    explicit SettingsController(QObject* parent = nullptr);

    QString apiKey() const;
    void setApiKey(const QString& apiKey);

    QString providerName() const;
    QString statusMessage() const;

    Q_INVOKABLE void save();
    Q_INVOKABLE void load();

signals:
    void apiKeyChanged();
    void statusMessageChanged();

private:
    QString m_apiKey;
    QString m_statusMessage;
};