"""Saved portfolios controller."""

from PySide6.QtCore import QObject, Property, Signal, Slot
from ..services.data_store import DataStore


class SavedController(QObject):
    portfoliosChanged = Signal()

    def __init__(self, data_store: DataStore, parent=None):
        super().__init__(parent)
        self._data_store = data_store
        self._portfolios = []
        self.refresh()

    def _get_portfolios(self) -> list:
        return self._portfolios

    portfolios = Property("QVariant", _get_portfolios, notify=portfoliosChanged)

    @Slot()
    def refresh(self):
        raw = self._data_store.load_portfolios()
        self._portfolios = []
        for p in raw:
            holdings = [{"ticker": h.ticker, "weight": h.weight, "yield": h.dividend_yield, "beta": h.beta} for h in p.holdings]
            self._portfolios.append({
                "id": p.id,
                "name": p.name,
                "targetYield": p.target_yield,
                "achievedYield": p.achieved_yield,
                "aggregateBeta": p.aggregate_beta,
                "holdings": holdings,
                "createdAt": p.created_at.strftime("%Y-%m-%d %H:%M") if p.created_at else "",
            })
        self.portfoliosChanged.emit()

    @Slot(str)
    def deletePortfolio(self, portfolio_id: str):
        self._data_store.delete_portfolio(portfolio_id)
        self.refresh()

    @Slot(str, str)
    def renamePortfolio(self, portfolio_id: str, new_name: str):
        portfolios = self._data_store.load_portfolios()
        for p in portfolios:
            if p.id == portfolio_id:
                p.name = new_name
                self._data_store.save_portfolio(p)
                break
        self.refresh()
