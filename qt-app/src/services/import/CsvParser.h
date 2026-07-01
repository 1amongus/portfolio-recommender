#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

#include "../../models/Portfolio.h"

struct ParseResult {
    QVector<Holding> holdings;
    QStringList errors;
    QStringList warnings;
};

class CsvParser {
public:
    ParseResult parse(const QString& filePath) const;
    ParseResult parseText(const QString& csvText) const;

private:
    ParseResult processLines(const QStringList& lines) const;
};
