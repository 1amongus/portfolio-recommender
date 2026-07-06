#include <QCoreApplication>
#include <QApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSystemTrayIcon>
#include <QWindow>

#include "../services/data/DataStore.h"
#include "../services/data/UniverseManager.h"
#include "../ui/controllers/BacktestController.h"
#include "../ui/controllers/ImportController.h"
#include "../ui/controllers/PortfolioController.h"
#include "../ui/controllers/SavedController.h"
#include "../ui/controllers/SensitivityController.h"
#include "../ui/controllers/SettingsController.h"
#include "TrayManager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("Portfolio Recommender"));
    app.setApplicationVersion(QStringLiteral("1.0.0"));
    app.setOrganizationName(QStringLiteral("PortfolioRecommender"));
    app.setOrganizationDomain(QStringLiteral("portfoliorecommender.local"));

    qmlRegisterType<PortfolioController>("PortfolioRecommender.Controllers", 1, 0, "PortfolioController");
    qmlRegisterType<ImportController>("PortfolioRecommender.Controllers", 1, 0, "ImportController");
    qmlRegisterType<SensitivityController>("PortfolioRecommender.Controllers", 1, 0, "SensitivityController");
    qmlRegisterType<BacktestController>("PortfolioRecommender.Controllers", 1, 0, "BacktestController");
    qmlRegisterType<SettingsController>("PortfolioRecommender.Controllers", 1, 0, "SettingsController");
    qmlRegisterType<SavedController>("PortfolioRecommender.Controllers", 1, 0, "SavedController");

    DataStore settingsDataStore;
    UniverseManager universeManager(&settingsDataStore);
    SettingsController settingsController(&universeManager, &settingsDataStore);
    BacktestController backtestController;
    ImportController importController;
    PortfolioController portfolioController;
    SavedController savedController;
    SensitivityController sensitivityController;
    TrayManager trayManager;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("backtestController"), &backtestController);
    engine.rootContext()->setContextProperty(QStringLiteral("importController"), &importController);
    engine.rootContext()->setContextProperty(QStringLiteral("portfolioController"), &portfolioController);
    engine.rootContext()->setContextProperty(QStringLiteral("savedController"), &savedController);
    engine.rootContext()->setContextProperty(QStringLiteral("sensitivityController"), &sensitivityController);
    engine.rootContext()->setContextProperty(QStringLiteral("settingsController"), &settingsController);
    engine.rootContext()->setContextProperty(QStringLiteral("trayManager"), &trayManager);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("PortfolioRecommender", "Main");
    app.setQuitOnLastWindowClosed(!QSystemTrayIcon::isSystemTrayAvailable());

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    if (auto* window = qobject_cast<QWindow*>(engine.rootObjects().constFirst())) {
        trayManager.setup(window);
    }

    QObject::connect(&trayManager, &TrayManager::generateRequested, &portfolioController, &PortfolioController::generate);
    QObject::connect(&trayManager, &TrayManager::quitRequested, &app, &QCoreApplication::quit);

    return app.exec();
}
