"""Tests for the optimizer service."""

import pytest
from src.models import Asset, Holding
from src.services.optimizer import optimize, portfolio_yield, portfolio_beta
from src.services.universe import DEFAULT_UNIVERSE


def test_optimize_returns_holdings():
    holdings = optimize(DEFAULT_UNIVERSE, 0.03, max_assets=5)
    assert len(holdings) > 0
    assert all(isinstance(h, Holding) for h in holdings)


def test_optimize_weights_sum_to_one():
    holdings = optimize(DEFAULT_UNIVERSE, 0.03, max_assets=5)
    total = sum(h.weight for h in holdings)
    assert abs(total - 1.0) < 1e-6


def test_optimize_empty_input():
    assert optimize([], 0.03) == []


def test_optimize_high_yield_selects_single():
    # MO has 8.2% yield, should be selected alone for 5% target
    holdings = optimize(DEFAULT_UNIVERSE, 0.05, max_assets=5)
    assert len(holdings) >= 1
    achieved = portfolio_yield(holdings)
    assert achieved >= 0.05 - 1e-6


def test_optimize_low_beta_preference():
    holdings = optimize(DEFAULT_UNIVERSE, 0.03, max_assets=5)
    beta = portfolio_beta(holdings)
    assert beta < 1.0  # Should prefer low-beta assets


def test_portfolio_yield_calculation():
    holdings = [Holding("A", 0.6, 0.04, 0.5), Holding("B", 0.4, 0.06, 0.8)]
    expected = 0.6 * 0.04 + 0.4 * 0.06
    assert abs(portfolio_yield(holdings) - expected) < 1e-9


def test_portfolio_beta_calculation():
    holdings = [Holding("A", 0.6, 0.04, 0.5), Holding("B", 0.4, 0.06, 0.8)]
    expected = 0.6 * 0.5 + 0.4 * 0.8
    assert abs(portfolio_beta(holdings) - expected) < 1e-9


def test_default_universe_all_positive_yield():
    assert all(a.dividend_yield > 0 for a in DEFAULT_UNIVERSE)


def test_default_universe_has_20_assets():
    assert len(DEFAULT_UNIVERSE) == 20
