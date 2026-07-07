"""Sensitivity controller - computes yield/beta trade-off curve."""

from PySide6.QtCore import QObject, Property, Signal, Slot
from ..services.data_store import DataStore
from ..services.optimizer import optimize, portfolio_yield, portfolio_beta


class SensitivityController(QObject):
    curveChanged = Signal()
    isComputingChanged = Signal()

    def __init__(self, data_store: DataStore, parent=None):
        super().__init__(parent)
        self._data_store = data_store
        self._is_computing = False
        self._curve = []

    def _get_is_computing(self) -> bool:
        return self._is_computing

    isComputing = Property(bool, _get_is_computing, notify=isComputingChanged)

    def _get_curve(self) -> list:
        return self._curve

    curve = Property("QVariant", _get_curve, notify=curveChanged)

    @Slot(float, float, float)
    def computeCurve(self, min_yield: float = 0.01, max_yield: float = 0.10, step: float = 0.005):
        """Compute sensitivity curve: for each target yield, find optimal beta."""
        if self._is_computing:
            return

        self._is_computing = True
        self.isComputingChanged.emit()

        assets = self._data_store.load_assets()
        points = []
        target = min_yield

        while target <= max_yield + 1e-9:
            holdings = optimize(assets, target, max_assets=5)
            if holdings:
                achieved = portfolio_yield(holdings)
                beta = portfolio_beta(holdings)
                points.append({
                    "targetYield": round(target * 100, 2),
                    "achievedYield": round(achieved * 100, 2),
                    "beta": round(beta, 4),
                })
            target += step

        self._curve = points
        self.curveChanged.emit()

        self._is_computing = False
        self.isComputingChanged.emit()
        print(f"[SensitivityController] Computed {len(points)} points")
