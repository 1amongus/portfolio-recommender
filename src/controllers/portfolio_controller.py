"""Controller bridging QML UI to the portfolio optimizer."""

from PySide6.QtCore import QObject, Property, Signal, Slot

from ..models import Portfolio
from ..services.data_store import DataStore
from ..services.optimizer import optimize, portfolio_yield, portfolio_beta
from ..services.universe import DEFAULT_UNIVERSE


class PortfolioController(QObject):
    targetYieldChanged = Signal()
    portfolioResultChanged = Signal()
    isLoadingChanged = Signal()
    errorMessageChanged = Signal()
    portfolioReady = Signal()

    def __init__(self, data_store: DataStore, parent=None):
        super().__init__(parent)
        self._data_store = data_store
        self._target_yield = 0.03
        self._is_loading = False
        self._error_message = ""
        self._portfolio_result = {}

        # Ensure seed data exists
        self._ensure_seed_data()

    def _ensure_seed_data(self):
        assets = self._data_store.load_assets()
        if len(assets) <= 6:
            existing_tickers = {a.ticker.upper() for a in assets}
            for seed in DEFAULT_UNIVERSE:
                if seed.ticker.upper() not in existing_tickers:
                    assets.append(seed)
            self._data_store.save_assets(assets)

    # --- Properties ---
    def _get_target_yield(self) -> float:
        return self._target_yield

    def _set_target_yield(self, value: float):
        if abs(self._target_yield - value) < 1e-9:
            return
        self._target_yield = value
        self.targetYieldChanged.emit()

    targetYield = Property(float, _get_target_yield, _set_target_yield, notify=targetYieldChanged)

    def _get_portfolio_result(self) -> dict:
        return self._portfolio_result

    portfolioResult = Property("QVariant", _get_portfolio_result, notify=portfolioResultChanged)

    def _get_is_loading(self) -> bool:
        return self._is_loading

    isLoading = Property(bool, _get_is_loading, notify=isLoadingChanged)

    def _get_error_message(self) -> str:
        return self._error_message

    errorMessage = Property(str, _get_error_message, notify=errorMessageChanged)

    # --- Slots ---
    @Slot()
    def generate(self):
        if self._is_loading:
            return

        print(f"[PortfolioController] generate() called, targetYield={self._target_yield}")

        self._is_loading = True
        self.isLoadingChanged.emit()
        self._set_error("")

        assets = self._data_store.load_assets()
        print(f"[PortfolioController] Loaded {len(assets)} assets")

        holdings = optimize(assets, self._target_yield, max_assets=5)
        print(f"[PortfolioController] Optimizer returned {len(holdings)} holdings")

        if not holdings:
            self._portfolio_result = {}
            self.portfolioResultChanged.emit()
            self._set_error("No qualifying assets available for the requested yield.")
            self._is_loading = False
            self.isLoadingChanged.emit()
            return

        # Build result for QML
        holdings_list = []
        for h in holdings:
            holdings_list.append({
                "ticker": h.ticker,
                "weight": h.weight,
                "yield": h.dividend_yield,
                "beta": h.beta,
            })

        achieved = portfolio_yield(holdings)
        agg_beta = portfolio_beta(holdings)

        # Save portfolio
        portfolio = Portfolio(
            name="Generated Portfolio",
            target_yield=self._target_yield,
            achieved_yield=achieved,
            aggregate_beta=agg_beta,
            holdings=holdings,
        )
        self._data_store.save_portfolio(portfolio)

        self._portfolio_result = {
            "holdings": holdings_list,
            "achievedYield": achieved,
            "aggregateBeta": agg_beta,
            "targetYield": self._target_yield,
            "name": portfolio.name,
        }
        self.portfolioResultChanged.emit()

        self._is_loading = False
        self.isLoadingChanged.emit()
        self.portfolioReady.emit()
        print(f"[PortfolioController] Done: yield={achieved:.4f}, beta={agg_beta:.4f}")

    @Slot(float)
    def setTargetYield(self, value: float):
        self._set_target_yield(value)

    @Slot("QVariant")
    def loadPortfolio(self, portfolio_data):
        """Load a saved portfolio into the result view."""
        data = portfolio_data if isinstance(portfolio_data, dict) else portfolio_data.toVariant()
        holdings = data.get("holdings", [])
        if not holdings:
            self._set_error("No holdings in portfolio.")
            return

        self._target_yield = data.get("targetYield", 0.03)
        self.targetYieldChanged.emit()

        self._portfolio_result = {
            "holdings": holdings,
            "achievedYield": data.get("achievedYield", 0),
            "aggregateBeta": data.get("aggregateBeta", 0),
            "targetYield": self._target_yield,
            "name": data.get("name", "Loaded Portfolio"),
        }
        self.portfolioResultChanged.emit()
        self._set_error("")
        self.portfolioReady.emit()

    def _set_error(self, msg: str):
        if self._error_message == msg:
            return
        self._error_message = msg
        self.errorMessageChanged.emit()
