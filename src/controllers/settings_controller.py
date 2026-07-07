"""Settings controller for API keys and universe management."""

from PySide6.QtCore import QObject, Property, Signal, Slot, QSettings

from ..services.data_store import DataStore
from ..services.universe import DEFAULT_UNIVERSE


class SettingsController(QObject):
    apiKeyChanged = Signal()
    universeChanged = Signal()

    def __init__(self, data_store: DataStore, parent=None):
        super().__init__(parent)
        self._data_store = data_store
        self._settings = QSettings()

    def _get_alpha_vantage_key(self) -> str:
        return self._settings.value("marketData/alphaVantageApiKey", "")

    def _set_alpha_vantage_key(self, key: str):
        self._settings.setValue("marketData/alphaVantageApiKey", key)
        self.apiKeyChanged.emit()

    alphaVantageApiKey = Property(str, _get_alpha_vantage_key, _set_alpha_vantage_key, notify=apiKeyChanged)

    @Slot(str)
    def setAlphaVantageApiKey(self, key: str):
        self._set_alpha_vantage_key(key)

    @Slot(result="QVariant")
    def getAssets(self):
        assets = self._data_store.load_assets()
        return [{"ticker": a.ticker, "name": a.name, "sector": a.sector} for a in assets]

    @Slot()
    def resetUniverse(self):
        self._data_store.save_assets(list(DEFAULT_UNIVERSE))
        self.universeChanged.emit()

    @Slot(str, str, str)
    def addAsset(self, ticker: str, name: str, sector: str):
        from ..models import Asset
        assets = self._data_store.load_assets()
        if any(a.ticker.upper() == ticker.upper() for a in assets):
            return
        assets.append(Asset(ticker=ticker.upper(), name=name, sector=sector))
        self._data_store.save_assets(assets)
        self.universeChanged.emit()

    @Slot(str)
    def removeAsset(self, ticker: str):
        assets = self._data_store.load_assets()
        assets = [a for a in assets if a.ticker.upper() != ticker.upper()]
        self._data_store.save_assets(assets)
        self.universeChanged.emit()
