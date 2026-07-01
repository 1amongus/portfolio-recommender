#include "Logger.h"

#include <QDateTime>
#include <QDebug>

namespace {
QString formatMessage(const QString& level, const QString& category, const QString& message)
{
    return QStringLiteral("[%1] [%2] [%3] %4")
        .arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs), level, category, message);
}
}

void Logger::info(const QString& category, const QString& message)
{
    qDebug().noquote() << formatMessage(QStringLiteral("INFO"), category, message);
}

void Logger::warning(const QString& category, const QString& message)
{
    qWarning().noquote() << formatMessage(QStringLiteral("WARN"), category, message);
}

void Logger::error(const QString& category, const QString& message)
{
    qCritical().noquote() << formatMessage(QStringLiteral("ERROR"), category, message);
}