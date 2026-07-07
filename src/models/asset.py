from dataclasses import dataclass, field
from datetime import datetime


@dataclass
class Asset:
    ticker: str
    name: str = ""
    sector: str = ""
    price: float = 0.0
    dividend_yield: float = 0.0
    beta: float = 0.0
    market_cap: float = 0.0
    is_etf: bool = False
    last_updated: datetime = field(default_factory=datetime.utcnow)

    @property
    def is_stale(self) -> bool:
        if self.last_updated is None:
            return True
        from datetime import timedelta
        return (datetime.utcnow() - self.last_updated) > timedelta(days=30)
