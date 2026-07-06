#pragma once

#include <QJsonObject>
#include <QString>
#include <QStringList>

struct ValidationResult {
    bool valid = false;
    QStringList errors;
};

class ResponseValidator
{
public:
    static ValidationResult validateAlphaVantageOverview(const QJsonObject& json);
    static ValidationResult validateAlphaVantageTimeSeries(const QJsonObject& json);
    static ValidationResult validateFmpProfile(const QJsonObject& json);
    static ValidationResult validatePolygonAggs(const QJsonObject& json);
    static ValidationResult validateFredObservations(const QJsonObject& json);
};
