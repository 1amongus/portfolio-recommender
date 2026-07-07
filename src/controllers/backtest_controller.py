"""Backtest controller - simulates historical portfolio performance."""

from PySide6.QtCore import QObject, Property, Signal, Slot
from ..services.data_store import DataStore


class BacktestController(QObject):
    resultChanged = Signal()
    isRunningChanged = Signal()

    def __init__(self, data_store: DataStore, parent=None):
        super().__init__(parent)
        self._data_store = data_store
        self._is_running = False
        self._result = {}

    def _get_is_running(self) -> bool:
        return self._is_running

    isRunning = Property(bool, _get_is_running, notify=isRunningChanged)

    def _get_result(self) -> dict:
        return self._result

    result = Property("QVariant", _get_result, notify=resultChanged)

    @Slot("QVariant", int)
    def runBacktest(self, holdings_data, months: int = 12):
        """Run a simple backtest simulation on the given holdings."""
        if self._is_running:
            return

        self._is_running = True
        self.isRunningChanged.emit()

        import random
        random.seed(42)

        holdings = holdings_data if isinstance(holdings_data, list) else holdings_data.toVariant()
        if not holdings:
            self._result = {"error": "No holdings to backtest"}
            self.resultChanged.emit()
            self._is_running = False
            self.isRunningChanged.emit()
            return

        # Simulate monthly returns based on beta and yield
        monthly_data = []
        cumulative = 1.0
        for month in range(months):
            market_return = random.gauss(0.008, 0.04)  # ~10% annual, 16% vol
            portfolio_return = 0.0
            for h in holdings:
                weight = h.get("weight", 0) if isinstance(h, dict) else h.property("weight")
                beta = h.get("beta", 1.0) if isinstance(h, dict) else h.property("beta")
                div_yield = h.get("yield", 0) if isinstance(h, dict) else h.property("yield")
                monthly_div = div_yield / 12.0
                stock_return = beta * market_return + monthly_div
                portfolio_return += weight * stock_return

            cumulative *= (1 + portfolio_return)
            monthly_data.append({
                "month": month + 1,
                "return": round(portfolio_return * 100, 2),
                "cumulative": round((cumulative - 1) * 100, 2),
            })

        total_return = (cumulative - 1) * 100
        annualized = ((cumulative ** (12.0 / months)) - 1) * 100 if months > 0 else 0
        max_drawdown = 0.0
        peak = 0.0
        for pt in monthly_data:
            if pt["cumulative"] > peak:
                peak = pt["cumulative"]
            dd = peak - pt["cumulative"]
            if dd > max_drawdown:
                max_drawdown = dd

        self._result = {
            "monthlyData": monthly_data,
            "totalReturn": round(total_return, 2),
            "annualizedReturn": round(annualized, 2),
            "maxDrawdown": round(max_drawdown, 2),
            "months": months,
        }
        self.resultChanged.emit()

        self._is_running = False
        self.isRunningChanged.emit()
        print(f"[BacktestController] Done: {total_return:.2f}% over {months}mo")
