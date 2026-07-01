#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "../ui/controllers/BacktestController.h"
#include "../ui/controllers/ImportController.h"
#include "../ui/controllers/PortfolioController.h"
#include "../ui/controllers/SensitivityController.h"
#include "../ui/controllers/SettingsController.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("Portfolio Recommender"));
    app.setApplicationVersion(QStringLiteral("1.0.0"));
    app.setOrganizationName(QStringLiteral("PortfolioRecommender"));
    app.setOrganizationDomain(QStringLiteral("portfoliorecommender.local"));

    qmlRegisterType<PortfolioController>("PortfolioRecommender.Controllers", 1, 0, "PortfolioController");
    qmlRegisterType<ImportController>("PortfolioRecommender.Controllers", 1, 0, "ImportController");
    qmlRegisterType<SensitivityController>("PortfolioRecommender.Controllers", 1, 0, "SensitivityController");
    qmlRegisterType<BacktestController>("PortfolioRecommender.Controllers", 1, 0, "BacktestController");
    qmlRegisterType<SettingsController>("PortfolioRecommender.Controllers", 1, 0, "SettingsController");

    BacktestController backtestController;
    ImportController importController;
    PortfolioController portfolioController;
    SensitivityController sensitivityController;
    SettingsController settingsController;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("backtestController"), &backtestController);
    engine.rootContext()->setContextProperty(QStringLiteral("importController"), &importController);
    engine.rootContext()->setContextProperty(QStringLiteral("portfolioController"), &portfolioController);
    engine.rootContext()->setContextProperty(QStringLiteral("sensitivityController"), &sensitivityController);
    engine.rootContext()->setContextProperty(QStringLiteral("settingsController"), &settingsController);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("PortfolioRecommender", "Main");
    return app.exec();
}
