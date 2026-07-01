# Portfolio Recommender

A modern Electron desktop application that helps investors evaluate and optimize their portfolios using a data-driven, yield-targeted approach.

## Features

- **Yield-Targeted Portfolio Generation** — Specify a desired dividend yield and receive an optimized portfolio of ≤5 assets with minimized Beta
- **Historical Back-Testing** — Evaluate how recommended portfolios would have performed historically
- **Portfolio Import & Rebalance** — Import existing holdings and generate optimized alternatives
- **Yield & Beta Sensitivity Curve** — Visualize trade-offs between income and volatility
- **Local SLM Explanations** — AI-powered portfolio insights running entirely on-device

## Tech Stack

| Layer | Technology |
|-------|-----------|
| Shell | Electron 32+ |
| Frontend | React 18, TypeScript 5.5, Vite |
| State | Zustand |
| Styling | Tailwind CSS |
| Charts | Recharts |
| Storage | File-based JSON (electron-store, lowdb, atomic fs) |
| SLM Runtime | ONNX Runtime (planned) |
| Testing | Vitest, Playwright |

## Data Providers

| Provider | Role |
|----------|------|
| Alpha Vantage | Primary — fundamentals, daily prices, Beta |
| Financial Modeling Prep | Fallback — same scope |
| Polygon.io | Bulk historical price backfill |
| FRED | Risk-free rate (10Y Treasury) |

## Getting Started

### Prerequisites

- Node.js 20+
- npm 10+
- API keys for at least Alpha Vantage (free tier)

### Installation

```bash
git clone https://github.com/1amongus/portfolio-recommender.git
cd portfolio-recommender
npm install
```

### Development

```bash
npm run dev
```

### Build

```bash
npm run build
```

### Testing

```bash
npm test              # Unit tests
npm run test:e2e      # End-to-end tests
npm run typecheck     # Type checking
npm run lint          # Linting
```

## Project Structure

```
src/
├── main/               # Electron main process
│   ├── ipc/            # IPC handler registration
│   ├── services/       # Business logic
│   │   ├── optimization/   # Beta-minimization engine
│   │   ├── data/           # Providers, cache, storage
│   │   ├── backtest/       # Historical simulation
│   │   └── slm/            # Local language model
│   └── utils/          # Financial math, logging
├── renderer/           # React frontend
│   ├── pages/          # Route pages
│   ├── components/     # Reusable UI
│   ├── stores/         # Zustand state
│   └── hooks/          # Custom hooks
├── shared/             # Shared types & validation
│   ├── types/          # TypeScript interfaces
│   └── validation/     # Zod schemas
└── preload/            # Electron contextBridge
```

## Architecture

- **Main process** handles all business logic, API calls, optimization, and file I/O
- **Renderer process** is a pure React UI communicating exclusively via typed IPC
- **No database engine** — all persistence uses human-readable JSON files with atomic writes
- **Security-first** — CSP enforced, HTTPS-only, OS credential storage, no telemetry

## License

MIT
