"""Persistent JSON storage for assets, portfolios, and cache."""

import json
import os
from datetime import datetime
from pathlib import Path
from typing import List, Optional

from PySide6.QtCore import QStandardPaths

from ..models import Asset, Holding, Portfolio


def _data_dir() -> Path:
    path = Path(QStandardPaths.writableLocation(QStandardPaths.StandardLocation.AppDataLocation))
    path.mkdir(parents=True, exist_ok=True)
    return path


def _atomic_write(filepath: Path, content: str) -> None:
    tmp = filepath.with_suffix(".tmp")
    tmp.write_text(content, encoding="utf-8")
    if filepath.exists():
        filepath.unlink()
    tmp.rename(filepath)


class DataStore:
    def __init__(self):
        self._dir = _data_dir()

    @property
    def data_directory(self) -> Path:
        return self._dir

    # --- Assets ---
    def load_assets(self) -> List[Asset]:
        path = self._dir / "assets.json"
        if not path.exists():
            return []
        try:
            data = json.loads(path.read_text(encoding="utf-8"))
            return [self._asset_from_dict(d) for d in data]
        except (json.JSONDecodeError, KeyError):
            return []

    def save_assets(self, assets: List[Asset]) -> None:
        path = self._dir / "assets.json"
        data = [self._asset_to_dict(a) for a in assets]
        _atomic_write(path, json.dumps(data, indent=2))

    # --- Portfolios ---
    def load_portfolios(self) -> List[Portfolio]:
        path = self._dir / "portfolios.json"
        if not path.exists():
            return []
        try:
            data = json.loads(path.read_text(encoding="utf-8"))
            return [self._portfolio_from_dict(d) for d in data]
        except (json.JSONDecodeError, KeyError):
            return []

    def save_portfolio(self, portfolio: Portfolio) -> None:
        portfolios = self.load_portfolios()
        existing = next((i for i, p in enumerate(portfolios) if p.id == portfolio.id), None)
        if existing is not None:
            portfolios[existing] = portfolio
        else:
            portfolios.append(portfolio)
        path = self._dir / "portfolios.json"
        data = [self._portfolio_to_dict(p) for p in portfolios]
        _atomic_write(path, json.dumps(data, indent=2))

    def delete_portfolio(self, portfolio_id: str) -> bool:
        portfolios = self.load_portfolios()
        filtered = [p for p in portfolios if p.id != portfolio_id]
        if len(filtered) == len(portfolios):
            return False
        path = self._dir / "portfolios.json"
        _atomic_write(path, json.dumps([self._portfolio_to_dict(p) for p in filtered], indent=2))
        return True

    # --- Serialization helpers ---
    @staticmethod
    def _asset_to_dict(asset: Asset) -> dict:
        return {
            "ticker": asset.ticker,
            "name": asset.name,
            "sector": asset.sector,
            "price": asset.price,
            "dividendYield": asset.dividend_yield,
            "beta": asset.beta,
            "marketCap": asset.market_cap,
            "isETF": asset.is_etf,
            "lastUpdated": asset.last_updated.isoformat() if asset.last_updated else None,
        }

    @staticmethod
    def _asset_from_dict(d: dict) -> Asset:
        last_updated = None
        if d.get("lastUpdated"):
            try:
                last_updated = datetime.fromisoformat(d["lastUpdated"])
            except ValueError:
                last_updated = datetime.utcnow()
        return Asset(
            ticker=d.get("ticker", ""),
            name=d.get("name", ""),
            sector=d.get("sector", ""),
            price=d.get("price", 0.0),
            dividend_yield=d.get("dividendYield", 0.0),
            beta=d.get("beta", 0.0),
            market_cap=d.get("marketCap", 0.0),
            is_etf=d.get("isETF", False),
            last_updated=last_updated or datetime.utcnow(),
        )

    @staticmethod
    def _holding_to_dict(h: Holding) -> dict:
        return {"ticker": h.ticker, "weight": h.weight, "yield": h.dividend_yield, "beta": h.beta}

    @staticmethod
    def _holding_from_dict(d: dict) -> Holding:
        return Holding(
            ticker=d.get("ticker", ""),
            weight=d.get("weight", 0.0),
            dividend_yield=d.get("yield", 0.0),
            beta=d.get("beta", 0.0),
        )

    @staticmethod
    def _portfolio_to_dict(p: Portfolio) -> dict:
        return {
            "id": p.id,
            "name": p.name,
            "targetYield": p.target_yield,
            "achievedYield": p.achieved_yield,
            "aggregateBeta": p.aggregate_beta,
            "holdings": [DataStore._holding_to_dict(h) for h in p.holdings],
            "createdAt": p.created_at.isoformat() if p.created_at else None,
        }

    @staticmethod
    def _portfolio_from_dict(d: dict) -> Portfolio:
        created_at = None
        if d.get("createdAt"):
            try:
                created_at = datetime.fromisoformat(d["createdAt"])
            except ValueError:
                created_at = datetime.utcnow()
        holdings = [DataStore._holding_from_dict(h) for h in d.get("holdings", [])]
        return Portfolio(
            id=d.get("id", ""),
            name=d.get("name", ""),
            target_yield=d.get("targetYield", 0.0),
            achieved_yield=d.get("achievedYield", 0.0),
            aggregate_beta=d.get("aggregateBeta", 0.0),
            holdings=holdings,
            created_at=created_at or datetime.utcnow(),
        )
