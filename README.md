# Portfolio Recommender

A native desktop app for yield-targeted portfolio optimization with beta minimization.

## Tech Stack

- **Python 3.12** + **PySide6** (Qt 6 bindings)
- **QML** for the UI (Material Dark theme)
- Cross-platform: Windows, macOS, Linux

## Quick Start

```bash
pip install -r requirements.txt
python main.py
```

## Features

- Generate - Optimize portfolios for target dividend yield with minimal beta
- Backtest - Simulate historical performance over configurable periods
- Sensitivity - Compute yield vs. beta trade-off curves
- Import/Export - CSV import/export of assets and portfolios
- Saved - Browse, load, rename, delete saved portfolios
- Settings - API keys, asset universe management

## Architecture

```
main.py                     # Entry point
src/
  models/                   # Asset, Holding, Portfolio dataclasses
  services/                 # DataStore (JSON), Optimizer, Universe
  controllers/              # PySide6 QObject controllers (bridge to QML)
  ui/qml/                   # QML UI pages
tests/                      # pytest test suite
```

## Running Tests

```bash
python -m pytest tests/ -v
```
