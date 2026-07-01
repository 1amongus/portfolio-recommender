#include "SettingsController.h"

#include <QSettings>

SettingsController::SettingsController(QObject* parent)
    : QObject(parent)
{
    load();
}

QString SettingsController::apiKey() const
{
    return m_apiKey;
}

void SettingsController::setApiKey(const QString& apiKey)
{
    if (m_apiKey == apiKey) {
        return;
    }

    m_apiKey = apiKey;
    emit apiKeyChanged();
}

QString SettingsController::providerName() const
{
    return QStringLiteral("Alpha Vantage");
}

QString SettingsController::statusMessage() const
{
    return m_statusMessage;
}

void SettingsController::save()
{
    QSettings settings;
    settings.setValue(QStringLiteral("marketData/alphaVantageApiKey"), m_apiKey);
    m_statusMessage = QStringLiteral("Settings saved.");
    emit statusMessageChanged();
}

void SettingsController::load()
{
    QSettings settings;
    setApiKey(settings.value(QStringLiteral("marketData/alphaVantageApiKey")).toString());
    m_statusMessage = QStringLiteral("Settings loaded.");
    emit statusMessageChanged();
}