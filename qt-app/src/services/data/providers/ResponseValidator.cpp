#include "ResponseValidator.h"

#include <QJsonArray>
#include <QJsonValue>
#include <QLoggingCategory>

namespace {
bool isNumericValue(const QJsonValue& value)
{
    if (value.isDouble()) {
        return true;
    }

    bool ok = false;
    value.toString().toDouble(&ok);
    return ok;
}

ValidationResult finalizeValidation(const QString& name, QStringList errors)
{
    if (!errors.isEmpty()) {
        qWarning().noquote() << name << "response validation failed:" << errors.join(QStringLiteral("; "));
    }

    return {errors.isEmpty(), errors};
}

void requireObjectField(const QJsonObject& json, const QString& field, QStringList& errors)
{
    if (!json.value(field).isObject()) {
        errors.append(QStringLiteral("%1 must be an object").arg(field));
    }
}

void requireArrayField(const QJsonObject& json, const QString& field, QStringList& errors)
{
    if (!json.value(field).isArray()) {
        errors.append(QStringLiteral("%1 must be an array").arg(field));
    }
}

void requireStringField(const QJsonObject& json, const QString& field, QStringList& errors)
{
    if (!json.value(field).isString()) {
        errors.append(QStringLiteral("%1 must be a string").arg(field));
    }
}

void requireNumberField(const QJsonObject& json, const QString& field, QStringList& errors)
{
    if (!isNumericValue(json.value(field))) {
        errors.append(QStringLiteral("%1 must be numeric").arg(field));
    }
}

void requireBoolField(const QJsonObject& json, const QString& field, QStringList& errors)
{
    if (!json.value(field).isBool()) {
        errors.append(QStringLiteral("%1 must be a boolean").arg(field));
    }
}
}

ValidationResult ResponseValidator::validateAlphaVantageOverview(const QJsonObject& json)
{
    QStringList errors;
    requireStringField(json, QStringLiteral("Name"), errors);
    requireStringField(json, QStringLiteral("Sector"), errors);
    requireNumberField(json, QStringLiteral("DividendYield"), errors);
    requireNumberField(json, QStringLiteral("Beta"), errors);
    requireNumberField(json, QStringLiteral("MarketCapitalization"), errors);
    requireStringField(json, QStringLiteral("AssetType"), errors);
    return finalizeValidation(QStringLiteral("Alpha Vantage overview"), errors);
}

ValidationResult ResponseValidator::validateAlphaVantageTimeSeries(const QJsonObject& json)
{
    QStringList errors;
    requireObjectField(json, QStringLiteral("Time Series (Daily)"), errors);
    return finalizeValidation(QStringLiteral("Alpha Vantage time series"), errors);
}

ValidationResult ResponseValidator::validateFmpProfile(const QJsonObject& json)
{
    QStringList errors;
    requireStringField(json, QStringLiteral("companyName"), errors);
    requireStringField(json, QStringLiteral("sector"), errors);
    requireNumberField(json, QStringLiteral("price"), errors);
    requireNumberField(json, QStringLiteral("lastDiv"), errors);
    requireNumberField(json, QStringLiteral("beta"), errors);
    requireNumberField(json, QStringLiteral("mktCap"), errors);
    requireBoolField(json, QStringLiteral("isEtf"), errors);
    return finalizeValidation(QStringLiteral("FMP profile"), errors);
}

ValidationResult ResponseValidator::validatePolygonAggs(const QJsonObject& json)
{
    QStringList errors;
    requireArrayField(json, QStringLiteral("results"), errors);
    return finalizeValidation(QStringLiteral("Polygon aggregates"), errors);
}

ValidationResult ResponseValidator::validateFredObservations(const QJsonObject& json)
{
    QStringList errors;
    requireArrayField(json, QStringLiteral("observations"), errors);
    return finalizeValidation(QStringLiteral("FRED observations"), errors);
}
