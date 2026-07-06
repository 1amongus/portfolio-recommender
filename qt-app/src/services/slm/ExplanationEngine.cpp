#include "ExplanationEngine.h"

#include <QStringList>
#include <QtMath>

#include <algorithm>

namespace {
QString percentText(double value, int decimals = 2)
{
    return QStringLiteral("%1%").arg(QString::number(value * 100.0, 'f', decimals));
}

QString absolutePercentText(double value, int decimals = 2)
{
    return percentText(qAbs(value), decimals);
}

QString numberText(double value, int decimals = 2)
{
    return QString::number(value, 'f', decimals);
}

QString betaDescription(double beta)
{
    if (beta < 0.5) {
        return QStringLiteral("very low");
    }
    if (beta < 0.8) {
        return QStringLiteral("moderate");
    }
    if (beta <= 1.0) {
        return QStringLiteral("near-market");
    }
    return QStringLiteral("high");
}

QString portfolioRiskNote(const QVector<Holding>& holdings, double portfolioBeta)
{
    if (holdings.isEmpty()) {
        return QStringLiteral("Add holdings data to evaluate diversification and concentration risk in more detail.");
    }

    QVector<Holding> sorted = holdings;
    std::sort(sorted.begin(), sorted.end(), [](const Holding& left, const Holding& right) {
        return left.weight > right.weight;
    });

    const double topWeight = sorted.first().weight;
    if (sorted.size() == 1) {
        return QStringLiteral("Because the portfolio relies on a single holding, company- or fund-specific shocks can dominate results and sector diversification is limited.");
    }

    if (topWeight >= 0.6) {
        return QStringLiteral("One position accounts for most of the capital, so concentration risk remains elevated even if the headline Beta looks controlled.");
    }

    if (sorted.size() >= 5 && portfolioBeta < 0.8) {
        return QStringLiteral("Using five holdings spreads risk more effectively across positions, giving the portfolio a sturdier defensive profile; sector balance should still be reviewed separately.");
    }

    if (sorted.size() >= 4) {
        return QStringLiteral("The capital is spread across several holdings, which helps reduce single-name risk, although sector overlap may still influence outcomes.");
    }

    return QStringLiteral("With only a handful of holdings, returns and income are likely to be driven by a small core of positions, so diversification matters.");
}

QString performanceQuality(double sharpeRatio, double sortinoRatio)
{
    if (sharpeRatio >= 1.5 && sortinoRatio >= 2.0) {
        return QStringLiteral("strong risk-adjusted performance");
    }
    if (sharpeRatio >= 1.0 && sortinoRatio >= 1.3) {
        return QStringLiteral("solid risk-adjusted performance");
    }
    if (sharpeRatio >= 0.5) {
        return QStringLiteral("acceptable but not outstanding risk-adjusted performance");
    }
    return QStringLiteral("weaker risk-adjusted performance");
}

QString drawdownContext(double maxDrawdown)
{
    const double magnitude = qAbs(maxDrawdown);
    if (magnitude >= 0.30) {
        return QStringLiteral("during a severe market stress episode");
    }
    if (magnitude >= 0.15) {
        return QStringLiteral("during a meaningful correction");
    }
    return QStringLiteral("during a relatively contained pullback");
}

QString yieldCommentary(const BacktestMetrics& metrics)
{
    const QString averageYield = percentText(metrics.averageYield);
    const QString stability = percentText(metrics.yieldStability);

    if (metrics.averageYield >= 0.04 && metrics.yieldStability <= 0.005) {
        return QStringLiteral("Income generation stayed strong and comparatively steady, averaging %1 with only %2 of yield variability. A realized Beta near %3 keeps the return profile understandable relative to the market.")
            .arg(averageYield, stability, numberText(metrics.portfolioBeta, 2));
    }

    if (metrics.yieldStability <= 0.01) {
        return QStringLiteral("Yield held up reasonably well through the test window, averaging %1 with modest variability of %2. That suggests the income stream was fairly resilient even as prices moved.")
            .arg(averageYield, stability);
    }

    return QStringLiteral("The portfolio still produced about %1 of average yield, but variability of %2 suggests the income profile was less consistent and may require a larger margin of safety.")
        .arg(averageYield, stability);
}
}

ExplanationEngine::ExplanationEngine(QObject* parent)
    : QObject(parent)
{
}

QString ExplanationEngine::explainPortfolio(const QVector<Holding>& holdings,
                                            double targetYield,
                                            double achievedYield,
                                            double portfolioBeta) const
{
    QStringList sections;
    sections.append(QStringLiteral("This portfolio targets a %1 dividend yield and achieves %2 with a portfolio Beta of %3.")
                        .arg(percentText(targetYield), percentText(achievedYield), numberText(portfolioBeta, 2)));

    if (holdings.isEmpty()) {
        sections.append(QStringLiteral("No holdings were supplied, so the explanation can only describe the portfolio at an aggregate level."));
    } else {
        QVector<Holding> sorted = holdings;
        std::sort(sorted.begin(), sorted.end(), [](const Holding& left, const Holding& right) {
            return left.weight > right.weight;
        });

        const Holding& first = sorted.first();
        QString holdingSection =
            QStringLiteral("%1 (%2) anchors the portfolio with a %3 yield and Beta of %4, giving the allocation its main risk and income profile.")
                .arg(first.ticker,
                     percentText(first.weight),
                     percentText(first.yield),
                     numberText(first.beta, 2));

        if (sorted.size() > 1) {
            const Holding& second = sorted.at(1);
            holdingSection += QStringLiteral(" %1 (%2) adds further income at %3 while keeping Beta at %4.")
                                  .arg(second.ticker,
                                       percentText(second.weight),
                                       percentText(second.yield),
                                       numberText(second.beta, 2));
        }

        if (sorted.size() > 2) {
            holdingSection += QStringLiteral(" The remaining %1 position%2 broaden%3 the mix and help smooth single-name risk.")
                                  .arg(sorted.size() - 2)
                                  .arg(sorted.size() - 2 == 1 ? QString() : QStringLiteral("s"))
                                  .arg(sorted.size() - 2 == 1 ? QStringLiteral("s") : QString());
        }

        sections.append(holdingSection);
    }

    sections.append(QStringLiteral("The overall Beta of %1 indicates %2 market sensitivity compared to the S&P 500 (Beta = 1.0). This means the portfolio is expected to move approximately %3 as much as the broader market on average.")
                        .arg(numberText(portfolioBeta, 2),
                             betaDescription(portfolioBeta),
                             percentText(portfolioBeta, 0)));

    sections.append(portfolioRiskNote(holdings, portfolioBeta));
    return sections.join(QStringLiteral("\n\n"));
}

QString ExplanationEngine::explainBacktest(const BacktestMetrics& metrics, int years) const
{
    const int normalizedYears = qMax(1, years);

    return QStringLiteral("Over the %1-year backtest period, this portfolio delivered a cumulative return of %2 (%3 annualized).\n\n"
                          "Risk-adjusted performance: Sharpe ratio of %4 and Sortino ratio of %5, indicating %6. Maximum drawdown was %7, occurring %8.\n\n"
                          "%9")
        .arg(QString::number(normalizedYears),
             percentText(metrics.totalReturn),
             percentText(metrics.annualizedReturn),
             numberText(metrics.sharpeRatio, 2),
             numberText(metrics.sortinoRatio, 2),
             performanceQuality(metrics.sharpeRatio, metrics.sortinoRatio),
             absolutePercentText(metrics.maxDrawdown),
             drawdownContext(metrics.maxDrawdown),
             yieldCommentary(metrics));
}

QString ExplanationEngine::explainSensitivity(double minBeta, double maxBeta, double optimalYield) const
{
    const double lowBeta = qMin(minBeta, maxBeta);
    const double highBeta = qMax(minBeta, maxBeta);
    const double spread = highBeta - lowBeta;

    QString tradeoffInsight;
    if (spread <= 0.15) {
        tradeoffInsight = QStringLiteral("Beta changes only modestly across the tested range, so yield selection appears to have a limited impact on overall market sensitivity.");
    } else if (optimalYield <= 0.03) {
        tradeoffInsight = QStringLiteral("The lowest-risk point sits toward the lower-yield end of the range, implying that pushing for extra income likely comes with a visible jump in volatility.");
    } else if (optimalYield >= 0.05) {
        tradeoffInsight = QStringLiteral("The curve suggests you can still pursue a relatively generous yield before Beta meaningfully rises, which improves the income-versus-risk trade-off.");
    } else {
        tradeoffInsight = QStringLiteral("The middle of the yield range appears to offer the cleanest compromise between income and market sensitivity.");
    }

    return QStringLiteral("The sensitivity analysis reveals that achievable Beta ranges from %1 to %2 across your specified yield range. The optimal point for balancing income and risk appears near %3 yield where Beta is minimized.\n\n%4")
        .arg(numberText(lowBeta, 2),
             numberText(highBeta, 2),
             percentText(optimalYield),
             tradeoffInsight);
}

QString ExplanationEngine::explainRebalance(double originalBeta,
                                            double newBeta,
                                            double originalYield,
                                            double newYield) const
{
    const double deltaBeta = newBeta - originalBeta;
    const bool betaImproved = deltaBeta < 0.0;
    const double sensitivityChangePercent =
        qFuzzyIsNull(originalBeta) ? 0.0 : (qAbs(deltaBeta) / qAbs(originalBeta)) * 100.0;

    QString yieldChange;
    const double deltaYield = newYield - originalYield;
    if (qAbs(deltaYield) < 0.0005) {
        yieldChange = QStringLiteral("holding roughly steady");
    } else if (deltaYield > 0.0) {
        yieldChange = QStringLiteral("raising");
    } else {
        yieldChange = QStringLiteral("reducing");
    }

    QString recommendation;
    if (betaImproved && deltaYield >= 0.0) {
        recommendation = QStringLiteral("This is a favorable rebalance because it lowers risk while preserving or improving portfolio income.");
    } else if (betaImproved) {
        recommendation = QStringLiteral("The rebalance improves defensiveness, although the lower yield means the trade-off should be weighed against your income objective.");
    } else if (deltaYield > 0.0) {
        recommendation = QStringLiteral("The new mix pursues more income, but it does so by accepting greater market sensitivity.");
    } else {
        recommendation = QStringLiteral("This rebalance does not clearly improve either yield or Beta, so it may be worth revisiting the target or position mix.");
    }

    return QStringLiteral("Rebalancing %1 your portfolio Beta from %2 to %3 (a %4 %5 in market sensitivity) while %6 your dividend yield from %7 to %8.\n\n%9")
        .arg(betaImproved ? QStringLiteral("reduces") : QStringLiteral("changes"),
             numberText(originalBeta, 2),
             numberText(newBeta, 2),
             QString::number(sensitivityChangePercent, 'f', 1) + QLatin1Char('%'),
             betaImproved ? QStringLiteral("reduction") : QStringLiteral("increase"),
             yieldChange,
             percentText(originalYield),
             percentText(newYield),
             recommendation);
}

bool ExplanationEngine::isModelAvailable() const
{
    return false;
}
