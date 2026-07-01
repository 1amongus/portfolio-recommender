find_package(Qt6 REQUIRED COMPONENTS Concurrent)

qt_add_executable(PortfolioRecommenderApp
    app/main.cpp
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
    ui/controllers/PortfolioController.cpp
    ui/controllers/PortfolioController.h
    ui/controllers/SavedController.cpp
    ui/controllers/SavedController.h
    ui/controllers/ImportController.cpp
    ui/controllers/ImportController.h
    ui/controllers/BacktestController.cpp
    ui/controllers/BacktestController.h
    ui/controllers/SensitivityController.cpp
    ui/controllers/SensitivityController.h
    ui/controllers/SettingsController.cpp
    ui/controllers/SettingsController.h
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
        ui/qml/components/NavigationBar.qml
        ui/qml/pages/GeneratePage.qml
        ui/qml/pages/SettingsPage.qml
        ui/qml/pages/BackTestPage.qml
        ui/qml/pages/SensitivityPage.qml
        ui/qml/pages/ImportPage.qml
        ui/qml/pages/SavedPage.qml
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
)

set_target_properties(PortfolioRecommenderApp PROPERTIES
    WIN32_EXECUTABLE TRUE
)
