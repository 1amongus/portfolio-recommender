"""Portfolio Recommender - Main entry point."""

import sys
from pathlib import Path

from PySide6.QtCore import QCoreApplication, QUrl
from PySide6.QtGui import QGuiApplication
from PySide6.QtQml import QQmlApplicationEngine
from PySide6.QtQuickControls2 import QQuickStyle

from src.services.data_store import DataStore
from src.controllers.portfolio_controller import PortfolioController
from src.controllers.settings_controller import SettingsController
from src.controllers.backtest_controller import BacktestController
from src.controllers.sensitivity_controller import SensitivityController
from src.controllers.saved_controller import SavedController
from src.controllers.import_export_controller import ImportExportController
from src.controllers.assistant_controller import AssistantController


def main():
    QCoreApplication.setOrganizationName("PortfolioRecommender")
    QCoreApplication.setApplicationName("Portfolio Recommender")
    QCoreApplication.setApplicationVersion("2.0.0")

    app = QGuiApplication(sys.argv)
    QQuickStyle.setStyle("Material")

    # Services
    data_store = DataStore()
    print(f"[main] Data directory: {data_store.data_directory}")

    # Controllers
    portfolio_controller = PortfolioController(data_store)
    settings_controller = SettingsController(data_store)
    backtest_controller = BacktestController(data_store)
    sensitivity_controller = SensitivityController(data_store)
    saved_controller = SavedController(data_store)
    import_export_controller = ImportExportController(data_store)
    assistant_controller = AssistantController()

    # Connect: auto-refresh saved list when a portfolio is generated
    portfolio_controller.portfolioReady.connect(saved_controller.refresh)

    # QML Engine
    engine = QQmlApplicationEngine()

    # Expose controllers to QML
    ctx = engine.rootContext()
    ctx.setContextProperty("portfolioController", portfolio_controller)
    ctx.setContextProperty("settingsController", settings_controller)
    ctx.setContextProperty("backtestController", backtest_controller)
    ctx.setContextProperty("sensitivityController", sensitivity_controller)
    ctx.setContextProperty("savedController", saved_controller)
    ctx.setContextProperty("importExportController", import_export_controller)
    ctx.setContextProperty("assistantController", assistant_controller)

    # Load main QML
    qml_dir = Path(__file__).parent / "src" / "ui" / "qml"
    qml_file = qml_dir / "Main.qml"
    engine.load(QUrl.fromLocalFile(str(qml_file)))

    if not engine.rootObjects():
        print("[main] ERROR: Failed to load QML")
        sys.exit(1)

    print("[main] App started successfully")
    sys.exit(app.exec())


if __name__ == "__main__":
    main()
