# Portfolio Recommender

A native desktop application for yield-targeted portfolio optimization with Beta minimization. Built with **Qt 6 / QML / C++17** for maximum performance and minimal resource usage.

## Features

- **Portfolio Generation** — Specify a target dividend yield and get an optimized portfolio of ≤5 stocks/ETFs with minimized market sensitivity (Beta)
- **Back-Testing** — Historical simulation with equity curves, Sharpe/Sortino ratios, max drawdown, and rolling yield metrics
- **Sensitivity Curve** — Interactive Yield→Beta visualization showing trade-offs across yield targets
- **Import & Rebalance** — Import existing portfolios via CSV or manual entry, rebalance to target yield with side-by-side comparison
- **Saved Portfolios** — Persistent storage with detail view and management
- **20-Asset Universe** — Curated dividend stocks/ETFs with live refresh capability via Alpha Vantage API
- **System Tray** — Minimize to tray, quick access menu

## Performance

| Metric | Value |
|--------|-------|
| Cold start | < 1 second |
| Memory usage | ~85-115 MB |
| Bundle size | ~15 MB (exe) |
| Test suite | 4 suites, all passing |

## Prerequisites

- Qt 6.7+ (with Charts, Quick, Network, Concurrent, Widgets modules)
- CMake 3.21+
- C++17 compiler (MSVC 19+, GCC 11+, Clang 14+)

## Build

```bash
# Configure
cmake -B build -DCMAKE_PREFIX_PATH=/path/to/Qt/6.7.3/msvc2019_64

# Build
cmake --build build --config Release

# Test
cd build && ctest --output-on-failure

# Run
./build/src/PortfolioRecommenderApp
```

### Windows (with Visual Studio)

```powershell
# From VS Developer Command Prompt
cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="C:\Qt\6.7.3\msvc2019_64"
cmake --build build
```

## Project Structure

```
src/
├── app/                    # Entry point, TrayManager
├── models/                 # Portfolio, Asset, BacktestResult, SensitivityPoint
├── services/
│   ├── optimization/       # Beta-minimization engine
│   ├── backtest/           # Historical simulation engine
│   ├── sensitivity/        # Yield→Beta curve generator
│   ├── import/             # CSV parser
│   └── data/               # DataStore, UniverseManager, RateLimiter, providers
├── ui/
│   ├── controllers/        # QML-exposed C++ controllers (5)
│   └── qml/                # QML pages (6) and components
└── utils/                  # Financial math, Logger
tests/
└── unit/                   # Qt Test: optimizer, math, backtest, csvparser
```

## Data Providers

| Provider | Role | Free Tier |
|----------|------|-----------|
| Alpha Vantage | Primary (fundamentals, prices) | 5 calls/min |
| Financial Modeling Prep | Fallback | 250 calls/day |
| Polygon.io | Bulk historical backfill | 5 calls/min |
| FRED | Risk-free rate (10Y Treasury) | Unlimited |

## License

MIT