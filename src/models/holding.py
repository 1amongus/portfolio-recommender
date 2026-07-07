from dataclasses import dataclass


@dataclass
class Holding:
    ticker: str
    weight: float = 0.0
    dividend_yield: float = 0.0
    beta: float = 0.0
