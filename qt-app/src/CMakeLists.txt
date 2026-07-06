find_package(Qt6 REQUIRED COMPONENTS Charts Concurrent Widgets PrintSupport)

qt_add_executable(PortfolioRecommenderApp
    app/main.cpp
    app/TrayManager.cpp
    app/TrayManager.h
    models/Portfolio.h
    models/Asset.h
    models/BacktestResult.h
    models/SensitivityPoint.h
    services/backtest/Engine.cpp
    services/backtest/Engine.h
    services/optimization/Optimizer.cpp
    services/optimization/Optimizer.h
    services/import/CsvParser.cpp
    services/import/CsvParser.h
    services/export/CsvExporter.cpp
    services/export/CsvExporter.h
    services/export/PdfExporter.cpp
    services/export/PdfExporter.h
    services/slm/ExplanationEngine.cpp
    services/slm/ExplanationEngine.h
    services/sensitivity/CurveGenerator.cpp
    services/sensitivity/CurveGenerator.h
    services/data/DataStore.cpp
    services/data/DataStore.h
    services/data/UniverseManager.cpp
    services/data/UniverseManager.h
    services/data/RateLimiter.cpp
    services/data/RateLimiter.h
    services/data/providers/IMarketDataProvider.h
    services/data/providers/AlphaVantageProvider.cpp
    services/data/providers/AlphaVantageProvider.h
    services/data/providers/FmpProvider.cpp
    services/data/providers/FmpProvider.h
    services/data/providers/PolygonProvider.cpp
    services/data/providers/PolygonProvider.h
    services/data/providers/FredProvider.cpp
    services/data/providers/FredProvider.h
    services/data/providers/ProviderBase.cpp
    services/data/providers/ProviderBase.h
    services/data/providers/ResponseValidator.cpp
    services/data/providers/ResponseValidator.h
    ui/controllers/PortfolioController.cpp
    ui/controllers/PortfolioController.h
    ui/controllers/SavedController.cpp
    ui/controllers/SavedController.h
    ui/controllers/ImportController.cpp
    ui/controllers/ImportController.h
    ui/controllers/BacktestController.cpp
    ui/controllers/BacktestController.h
    ui/controllers/ExplanationController.cpp
    ui/controllers/ExplanationController.h
    ui/controllers/SensitivityController.cpp
    ui/controllers/SensitivityController.h
    ui/controllers/SettingsController.cpp
    ui/controllers/SettingsController.h
    ui/controllers/ExportController.cpp
    ui/controllers/ExportController.h
    utils/Math.cpp
    utils/Math.h
    utils/Logger.cpp
    utils/Logger.h
)

qt_add_qml_module(PortfolioRecommenderApp
    URI PortfolioRecommender
    VERSION 1.0
    QML_FILES
        ui/qml/Main.qml
        ui/qml/components/ExplanationPanel.qml
        ui/qml/components/NavigationBar.qml
        ui/qml/pages/GeneratePage.qml
        ui/qml/pages/SettingsPage.qml
        ui/qml/pages/BackTestPage.qml
        ui/qml/pages/SensitivityPage.qml
        ui/qml/pages/ImportPage.qml
        ui/qml/pages/SavedPage.qml
        ui/qml/pages/SetupWizard.qml
)

target_include_directories(PortfolioRecommenderApp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(PortfolioRecommenderApp
    PRIVATE
        Qt6::Core
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Network
        Qt6::Charts
        Qt6::Concurrent
        Qt6::Widgets
        Qt6::PrintSupport
)

set_target_properties(PortfolioRecommenderApp PROPERTIES
    WIN32_EXECUTABLE TRUE
)
