# Portfolio Recommender

A **native Qt desktop application** that helps investors evaluate and optimize their portfolios using a data-driven, yield-targeted approach. Built with Qt 6 / QML / C++17 for maximum performance (~85MB RAM, <1s startup).

## Features

- **Yield-Targeted Portfolio Generation** — Specify a desired dividend yield and receive an optimized portfolio of ≤5 assets with minimized Beta
- **Historical Back-Testing** — Evaluate how recommended portfolios would have performed with equity curves, Sharpe/Sortino ratios, and drawdown metrics
- **Portfolio Import & Rebalance** — Import existing holdings via CSV and generate optimized alternatives with side-by-side comparison
- **Yield & Beta Sensitivity Curve** — Interactive Canvas chart showing trade-offs between income and volatility
- **Saved Portfolios** — Persistent storage with master-detail view and management
- **20-Asset Universe** — Curated dividend stocks/ETFs with live API refresh

## Tech Stack

| Layer | Technology |
|-------|-----------|
| Framework | Qt 6.7+ (Quick, Charts, Network, Concurrent, Widgets) |
| Language | C++17 |
| UI | QML with Material dark theme |
| Build | CMake 3.21+ / Ninja |
| Storage | QSettings + QJsonDocument + atomic file writes |
| Testing | Qt Test + CTest |
| Packaging | CPack + windeployqt |
| CI | GitHub Actions |

## Data Providers

| Provider | Role | Free Tier |
|----------|------|-----------|
| Alpha Vantage | Primary — fundamentals, daily prices, Beta | 5 calls/min |
| Financial Modeling Prep | Fallback — same scope | 250 calls/day |
| Polygon.io | Bulk historical price backfill | 5 calls/min |
| FRED | Risk-free rate (10Y Treasury) | Unlimited |

## Getting Started

### Prerequisites

- Qt 6.7+ with modules: Core, Quick, QuickControls2, Network, Charts, Concurrent, Widgets
- CMake 3.21+
- C++17 compiler (MSVC 19+, GCC 11+, Clang 14+)

### Build & Run

```bash
cd qt-app
cmake -B build -DCMAKE_PREFIX_PATH=/path/to/Qt/6.7.3/gcc_64
cmake --build build --config Release
cd build && ctest --output-on-failure    # Run tests
./src/PortfolioRecommenderApp            # Launch
```

### Windows (Visual Studio)

```powershell
cd qt-app
cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="C:\Qt\6.7.3\msvc2019_64"
cmake --build build
.\build\src\PortfolioRecommenderApp.exe
```

## Project Structure

```
qt-app/
├── src/
│   ├── app/                    # Entry point, TrayManager
│   ├── models/                 # Portfolio, Asset, BacktestResult, SensitivityPoint
│   ├── services/
│   │   ├── optimization/       # Beta-minimization engine
│   │   ├── backtest/           # Historical simulation
│   │   ├── sensitivity/        # Yield→Beta curve generator
│   │   ├── import/             # CSV parser
│   │   └── data/               # DataStore, UniverseManager, Providers, RateLimiter
│   ├── ui/
│   │   ├── controllers/        # QML-exposed C++ controllers
│   │   └── qml/                # Pages (6) + components
│   └── utils/                  # Financial math, Logger
├── tests/unit/                 # Qt Test suites (4)
└── packaging/                  # CPack/NSIS installer config
```

## Architecture

- **Native rendering** — No embedded browser; GPU-accelerated Qt Scene Graph
- **Signal/slot** architecture for loose coupling between services and UI
- **QtConcurrent** for non-blocking computation (backtest, sensitivity, API calls)
- **No database engine** — all persistence uses human-readable JSON with atomic writes
- **System tray** — minimize to tray with quick-access context menu
- **Security-first** — HTTPS-only API calls, QSettings for preferences, no telemetry

## Legacy

The `src/` directory at the repository root contains the original Electron prototype (deprecated). The active application is in `qt-app/`.

## License

MIT
