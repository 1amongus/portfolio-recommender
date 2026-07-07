"""Portfolio optimizer: yield-targeted allocation with beta minimization."""

from typing import List

from ..models import Asset, Holding


def portfolio_yield(holdings: List[Holding]) -> float:
    total = sum(h.weight * h.dividend_yield for h in holdings)
    return total


def portfolio_beta(holdings: List[Holding]) -> float:
    total = sum(h.weight * h.beta for h in holdings)
    return total


def optimize(assets: List[Asset], target_yield: float, max_assets: int = 5) -> List[Holding]:
    """Generate optimal portfolio holdings for given target yield.
    
    Strategy: select lowest-beta assets that meet yield target,
    then iteratively shift weight toward higher-yielding assets.
    """
    if not assets or max_assets <= 0:
        return []

    # Filter to dividend-paying assets only
    universe = [a for a in assets if a.dividend_yield > 0]
    if not universe:
        return []

    # Sort by beta ascending, dividend_yield descending as tiebreaker
    universe.sort(key=lambda a: (a.beta, -a.dividend_yield))

    # Find qualifying assets (yield >= target)
    qualifying = [a for a in universe if a.dividend_yield >= target_yield - 1e-6]

    if qualifying:
        # Single best asset: lowest beta among qualifying
        best = min(qualifying, key=lambda a: a.beta)
        return [Holding(best.ticker, 1.0, best.dividend_yield, best.beta)]

    # Take top N lowest-beta assets and optimize weights
    selection = universe[:max_assets]
    holdings = [Holding(a.ticker, 1.0 / len(selection), a.dividend_yield, a.beta) for a in selection]

    # Iteratively shift weight from low-yield to high-yield holdings
    for _ in range(250):
        current_yield = portfolio_yield(holdings)
        if current_yield + 1e-6 >= target_yield:
            break

        low_idx = min(range(len(holdings)), key=lambda i: holdings[i].dividend_yield)
        high_idx = max(range(len(holdings)), key=lambda i: holdings[i].dividend_yield)

        if low_idx == high_idx:
            break

        shift = min(0.02, holdings[low_idx].weight)
        if shift <= 0:
            break

        holdings[low_idx].weight -= shift
        holdings[high_idx].weight += shift

    # Normalize and clean up
    total_weight = sum(h.weight for h in holdings)
    if total_weight > 0:
        for h in holdings:
            h.weight /= total_weight

    holdings = [h for h in holdings if h.weight > 1e-6]
    holdings.sort(key=lambda h: -h.weight)
    return holdings
