# Implementation Plan: Portfolio Recommender

**Branch**: `feat/portfolio-recommender-core` | **Date**: 2026-07-01 | **Spec**: `.specify/memory/specs/portfolio-recommender.md`  
**Input**: Feature specification from `/specs/portfolio-recommender/spec.md`

## Summary

Build an Electron desktop application that accepts a target dividend yield, constructs an optimized portfolio of ≤5 assets using Beta-minimization constrained optimization, back-tests the portfolio historically, supports importing/rebalancing existing portfolios, renders Yield/Beta sensitivity curves, and provides SLM-powered natural-language explanations. The architecture follows a strict main-process/renderer-process separation with IPC bridging.

## Technical Context

**Language/Version**: TypeScript 5.5+, Node.js 20 LTS  
**Primary Dependencies**: Electron 32+, React 18, Vite, Zustand, Recharts, electron-store, lowdb, ONNX Runtime Node  
**Storage**: File-based JSON (electron-store for preferences, lowdb for collections, atomic fs.promises for large blobs) + OS Credential Manager (API keys)  
**Testing**: Vitest (unit + integration), Playwright (E2E)  
**Target Platform**: Windows, macOS, Linux (desktop)  
**Project Type**: Desktop application (Electron)  
**Performance Goals**: <3s cold start, <10s portfolio generation, <15s back-test (5yr), <45s sensitivity curve (30 steps)  
**Constraints**: <1GB memory (excl. model weights), offline-capable with cached data, no cloud LLM calls without opt-in  
**Scale/Scope**: Single-user local app, ~500-ticker asset universe, 10+ years historical data

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| Principle | Status | Notes |
|-----------|--------|-------|
| I. Electron Desktop-First | ✅ PASS | Core architecture is Electron with React renderer |
| II. Public API Consumer | ✅ PASS | Uses Alpha Vantage/Yahoo Finance behind provider interface |
| III. Local SLM Integration | ✅ PASS | ONNX Runtime for local inference, no cloud calls by default |
| IV. Test-First | ✅ PASS | Vitest + Playwright; TDD workflow enforced |
| V. Security & Privacy | ✅ PASS | OS credential storage, CSP, no data exfiltration |
| VI. Separation of Concerns | ✅ PASS | Main/renderer/shared architecture with IPC |
| VII. Simplicity | ✅ PASS | Progressive feature delivery; core engine first |

## Project Structure

### Documentation

```text
.specify/memory/specs/
├── portfolio-recommender.md    # Feature specification
├── plan.md                     # This file
├── research.md                 # Phase 0 output (API analysis, optimization library evaluation)
├── data-model.md               # Phase 1 output (file-based data model, entity relationships)
├── quickstart.md               # Phase 1 output (dev environment setup)
├── contracts/                  # Phase 1 output (IPC contracts, API provider interfaces)
└── tasks.md                    # Phase 2 output (/speckit.tasks)
```

### Source Code (repository root)

```text
portfolio-recommender/
├── package.json
├── electron-builder.yml
├── vite.config.ts
├── tsconfig.json
├── tsconfig.node.json
│
├── src/
│   ├── main/                          # Electron main process
│   │   ├── index.ts                   # App entry, window management, lifecycle
│   │   ├── ipc/                       # IPC handler registration
│   │   │   ├── portfolio.ipc.ts       # Portfolio generation IPC handlers
│   │   │   ├── backtest.ipc.ts        # Back-testing IPC handlers
│   │   │   ├── sensitivity.ipc.ts     # Sensitivity curve IPC handlers
│   │   │   ├── import.ipc.ts          # Portfolio import IPC handlers
│   │   │   └── slm.ipc.ts            # SLM inference IPC handlers
│   │   ├── services/                  # Business logic (main process)
│   │   │   ├── optimization/          # Beta-minimization engine
│   │   │   │   ├── optimizer.ts       # Constrained optimization solver
│   │   │   │   ├── constraints.ts     # Yield/Beta/weight constraints
│   │   │   │   └── universe.ts        # Asset universe management
│   │   │   ├── backtest/              # Historical simulation engine
│   │   │   │   ├── engine.ts          # Core back-test loop
│   │   │   │   ├── metrics.ts         # Sharpe, Sortino, drawdown, etc.
│   │   │   │   └── rebalancer.ts      # Periodic rebalancing logic
│   │   │   ├── sensitivity/           # Sensitivity curve computation
│   │   │   │   └── curve-generator.ts # Iterative yield-sweep optimizer
│   │   │   ├── data/                  # Data access layer
│   │   │   │   ├── providers/         # API provider implementations
│   │   │   │   │   ├── provider.interface.ts
│   │   │   │   │   ├── alpha-vantage.provider.ts
│   │   │   │   │   ├── fmp.provider.ts
│   │   │   │   │   ├── polygon.provider.ts
│   │   │   │   │   └── fred.provider.ts
│   │   │   │   ├── cache.ts           # TTL-based local cache
│   │   │   │   ├── store.ts           # lowdb collection manager (portfolios, assets, sensitivity)
│   │   │   │   └── blob-store.ts      # Atomic fs.promises read/write for backtests & cache
│   │   │   ├── slm/                   # SLM inference service
│   │   │   │   ├── inference.ts       # ONNX Runtime / llama.cpp wrapper
│   │   │   │   ├── model-manager.ts   # Download, validate, select models
│   │   │   │   ├── prompts.ts         # Prompt templates for explanations
│   │   │   │   └── fallback.ts        # Template-based fallback explanations
│   │   │   ├── import/                # Portfolio import & validation
│   │   │   │   ├── csv-parser.ts      # CSV parsing & normalization
│   │   │   │   └── validator.ts       # Ticker validation & fuzzy matching
│   │   │   └── secure-storage.ts      # OS credential manager wrapper
│   │   └── utils/
│   │       ├── logger.ts
│   │       └── math.ts                # Financial math utilities
│   │
│   ├── renderer/                      # Electron renderer process (React)
│   │   ├── index.html
│   │   ├── main.tsx                   # React entry point
│   │   ├── App.tsx                    # Root component + routing
│   │   ├── components/                # Reusable UI components
│   │   │   ├── charts/
│   │   │   │   ├── EquityCurve.tsx
│   │   │   │   ├── SensitivityChart.tsx
│   │   │   │   ├── DrawdownChart.tsx
│   │   │   │   └── YieldTimeChart.tsx
│   │   │   ├── portfolio/
│   │   │   │   ├── PortfolioCard.tsx
│   │   │   │   ├── AssetTable.tsx
│   │   │   │   ├── ComparisonView.tsx
│   │   │   │   └── ImportWizard.tsx
│   │   │   ├── slm/
│   │   │   │   ├── ExplanationPanel.tsx
│   │   │   │   └── StreamingText.tsx
│   │   │   └── common/
│   │   │       ├── LoadingSpinner.tsx
│   │   │       ├── ProgressBar.tsx
│   │   │       ├── ErrorBoundary.tsx
│   │   │       └── StaleBadge.tsx
│   │   ├── pages/
│   │   │   ├── GeneratePage.tsx       # Yield input → portfolio result
│   │   │   ├── BackTestPage.tsx       # Back-test configuration & results
│   │   │   ├── ImportPage.tsx         # Import & rebalance workflow
│   │   │   ├── SensitivityPage.tsx    # Sensitivity curve analysis
│   │   │   ├── SavedPage.tsx          # Saved portfolios list
│   │   │   └── SettingsPage.tsx       # API keys, model config, preferences
│   │   ├── stores/                    # Zustand state stores
│   │   │   ├── portfolio.store.ts
│   │   │   ├── backtest.store.ts
│   │   │   ├── sensitivity.store.ts
│   │   │   └── settings.store.ts
│   │   ├── hooks/                     # Custom React hooks
│   │   │   ├── useIpc.ts             # Generic IPC invoke wrapper
│   │   │   └── useStreamingSlm.ts    # SLM streaming hook
│   │   └── styles/
│   │       └── globals.css            # Tailwind base + custom tokens
│   │
│   ├── shared/                        # Shared between main & renderer
│   │   ├── types/
│   │   │   ├── portfolio.types.ts
│   │   │   ├── backtest.types.ts
│   │   │   ├── asset.types.ts
│   │   │   ├── sensitivity.types.ts
│   │   │   └── ipc.types.ts          # IPC channel names & payload types
│   │   ├── constants.ts
│   │   └── validation/
│   │       └── schemas.ts             # Zod schemas for runtime validation
│   │
│   └── preload/
│       └── index.ts                   # contextBridge API exposure
│
├── tests/
│   ├── unit/
│   │   ├── optimization/
│   │   │   ├── optimizer.test.ts
│   │   │   └── constraints.test.ts
│   │   ├── backtest/
│   │   │   ├── engine.test.ts
│   │   │   └── metrics.test.ts
│   │   ├── sensitivity/
│   │   │   └── curve-generator.test.ts
│   │   ├── import/
│   │   │   ├── csv-parser.test.ts
│   │   │   └── validator.test.ts
│   │   └── slm/
│   │       ├── inference.test.ts
│   │       └── fallback.test.ts
│   ├── integration/
│   │   ├── providers/
│   │   │   ├── alpha-vantage.integration.test.ts
│   │   │   ├── fmp.integration.test.ts
│   │   │   ├── polygon.integration.test.ts
│   │   │   └── fred.integration.test.ts
│   │   ├── ipc/
│   │   │   └── portfolio.ipc.integration.test.ts
│   │   └── db/
│   │       └── persistence.integration.test.ts
│   └── e2e/
│       ├── generate-portfolio.e2e.ts
│       ├── backtest-flow.e2e.ts
│       ├── import-rebalance.e2e.ts
│       └── sensitivity-curve.e2e.ts
│
├── resources/                         # Electron static assets
│   ├── icons/
│   └── prompts/                       # SLM prompt templates
│       ├── explain-portfolio.txt
│       └── summarize-backtest.txt
│
└── scripts/
    ├── download-model.ts              # SLM model download utility
    └── seed-universe.ts               # Seed asset universe from API
```

**Structure Decision**: Electron split-process architecture with `src/main`, `src/renderer`, `src/shared`, and `src/preload`. All business logic (optimization, back-testing, data access, SLM) lives in the main process to keep the renderer lightweight and avoid blocking the UI thread. IPC contracts in `shared/types/ipc.types.ts` serve as the single source of truth for communication between processes.

**Branching Strategy**: Multi-week phases use sub-feature branches (e.g., `feat/backtest-engine`, `feat/backtest-charts`) that are individually short-lived (< 1 week) per constitution. Sub-branches merge into `main` independently; no long-lived integration branches.

---

## Implementation Phases

### Phase 0: Foundation & Scaffold (Week 1)

**Objective**: Bootable Electron app with dev tooling, empty shell UI, and verified build pipeline.

| # | Task | Deliverable |
|---|------|-------------|
| 0.1 | Initialize Electron + Vite + React + TypeScript project | `package.json`, configs, dev/build scripts |
| 0.2 | Configure ESLint, Prettier, Tailwind CSS | Linting passes on empty project |
| 0.3 | Set up Vitest (unit) + Playwright (E2E) | Test runners execute with sample tests |
| 0.4 | Create main/renderer/shared/preload directory structure | Skeleton files compile |
| 0.5 | Implement IPC bridge (preload + contextBridge) | Renderer can invoke main process handlers |
| 0.6 | Set up file-based storage layer (electron-store for prefs, lowdb for collections, blob-store for backtests/cache) | userData directory structure creates on first launch |
| 0.7 | Configure electron-builder for all platforms | Produces unsigned dev builds |
| 0.8 | CI pipeline: lint → typecheck → audit → test → build → bundle-size check | GitHub Actions workflow green; `npm audit` passes; renderer bundle < 2MB gzip |

**Exit Criteria**: `npm run dev` launches Electron window with React shell; `npm run build` produces platform packages; all CI gates pass.

---

### Phase 1: Data Layer & API Integration (Week 2)

**Objective**: Reliable market data access with caching, provider abstraction, and asset universe.

| # | Task | Deliverable |
|---|------|-------------|
| 1.1 | Define `MarketDataProvider` interface (yield, Beta, price history, dividends, risk-free rate) | `provider.interface.ts` with full type contracts |
| 1.2 | Implement Alpha Vantage provider (primary — fundamentals, daily prices, company overview) | Fetches quotes, yields, Beta, historical prices |
| 1.3 | Implement Financial Modeling Prep provider (fallback — same scope as Alpha Vantage) | Same interface, alternative data source |
| 1.4 | Build TTL-based file cache layer (per-response JSON files in userData/cache/ with atomic writes) | API responses cached; stale data served offline |
| 1.5 | Implement secure storage wrapper (OS credential manager) | API keys encrypted at rest |
| 1.6 | Implement Polygon.io bulk history provider (grouped daily bars for backfill) | Full price history for back-testing seeded efficiently |
| 1.7 | Implement FRED provider (10Y Treasury risk-free rate for Sharpe/Sortino) | Risk-free rate auto-refreshed |
| 1.8 | Seed asset universe (S&P 500 + top 100 ETFs) | Universe stored in assets.json via lowdb; refreshable |
| 1.9 | Build Settings page with API key setup wizard (Alpha Vantage, FMP, Polygon, FRED keys) | First-run UX for credential entry |
| 1.10 | Write contract tests for all providers | Mocked + live integration tests |
| 1.11 | Implement per-provider rate limiter (token bucket algorithm respecting API tier limits) | Rate limiter enforced on all provider calls; burst protection tested |

**Exit Criteria**: App fetches real market data, caches it locally, survives API outage with cached data, and securely stores credentials.

---

### Phase 2: Optimization Engine (Week 3)

**Objective**: Core portfolio generation — accept yield target, return ≤5 assets with minimized Beta.

| # | Task | Deliverable |
|---|------|-------------|
| 2.1 | Research & select optimization library (quadprog, OSQP, or custom SLSQP) | Decision documented in research.md |
| 2.2 | Implement constraint builder (yield ≥ target, weights sum to 1, ≤5 assets, weights ≥ 0) | `constraints.ts` with unit tests |
| 2.3 | Implement optimizer core (minimize portfolio Beta subject to constraints) | `optimizer.ts` with unit tests |
| 2.4 | Implement universe filtering (exclude stale data, insufficient history) | `universe.ts` filters |
| 2.5 | Wire IPC handler for portfolio generation | Renderer can request portfolio via IPC |
| 2.6 | Build Generate page UI (yield input, loading state, result display) | `GeneratePage.tsx` + `PortfolioCard.tsx` |
| 2.7 | Handle edge cases (impossible yield, single-asset result, all high-Beta) | Warning/error UX |
| 2.8 | Validate optimizer accuracy against manual calculations | Test suite with known-answer problems |

**Exit Criteria**: User enters yield → receives optimized portfolio in <10s; edge cases handled gracefully; optimizer accuracy validated within ±0.5%.

---

### Phase 3: Back-Testing Engine (Week 4–5)

**Objective**: Full historical simulation with metrics, charts, and configurable rebalancing.

| # | Task | Deliverable |
|---|------|-------------|
| 3.1 | Implement back-test engine core (daily price replay, dividend reinvestment) | `engine.ts` |
| 3.2 | Implement metrics calculator (Sharpe, Sortino, max drawdown, annualized return, volatility) | `metrics.ts` with unit tests |
| 3.3 | Implement periodic rebalancer (monthly/quarterly/annually/never) | `rebalancer.ts` |
| 3.4 | Handle missing data (pre-IPO assets, delistings, splits via adjusted prices) | Edge case logic + tests |
| 3.5 | Wire IPC handler for back-test execution with progress reporting | Progressive updates to renderer |
| 3.6 | Build equity curve chart (Recharts, interactive zoom/pan) | `EquityCurve.tsx` |
| 3.7 | Build drawdown chart + rolling yield chart | `DrawdownChart.tsx`, `YieldTimeChart.tsx` |
| 3.8 | Build back-test summary table (metrics dashboard) | `BackTestPage.tsx` complete |
| 3.9 | Validate back-test against known benchmark returns (SPY 5yr) | Accuracy test suite |

**Exit Criteria**: Back-test runs in <15s for 5yr period; all metrics match manual validation; charts are interactive and responsive.

---

### Phase 4: Import, Rebalance & Comparison (Week 5–6)

**Objective**: Import existing portfolios, generate rebalanced alternatives, compare via side-by-side back-tests.

| # | Task | Deliverable |
|---|------|-------------|
| 4.1 | Implement CSV parser (ticker+shares, ticker+weight formats) | `csv-parser.ts` + tests |
| 4.2 | Implement ticker validator with fuzzy matching (Levenshtein) | `validator.ts` + tests |
| 4.3 | Build Import Wizard UI (upload, validate, correct, confirm) | `ImportWizard.tsx` |
| 4.4 | Implement manual entry mode with autocomplete | Ticker search in renderer |
| 4.5 | Wire rebalance to existing optimizer (imported → optimized) | IPC handler reuses Phase 2 engine |
| 4.6 | Build comparison view (original vs. rebalanced: metrics + overlaid charts) | `ComparisonView.tsx` |
| 4.7 | Run comparative back-tests with overlaid equity curves | Dual-series chart rendering |
| 4.8 | Export to CSV & PDF | File dialog + export logic |

**Exit Criteria**: User imports 50-holding portfolio in <5s; rebalanced portfolio generated; side-by-side back-test renders with clear improvement metrics.

---

### Phase 5: Sensitivity Curve (Week 6–7)

**Objective**: Yield vs. Beta efficient frontier visualization with interactive exploration.

| # | Task | Deliverable |
|---|------|-------------|
| 5.1 | Implement curve generator (iterative optimizer over yield range) | `curve-generator.ts` |
| 5.2 | Optimize computation (parallel yield steps, early termination for infeasible) | Performance within 45s budget |
| 5.3 | Wire IPC with per-step progress reporting | Progress bar in renderer |
| 5.4 | Build sensitivity chart (interactive, tooltips with portfolio composition) | `SensitivityChart.tsx` |
| 5.5 | Implement click-to-load (curve point → Generate page) | Navigation + state hydration |
| 5.6 | Plot imported portfolio marker on curve | Overlay current position |

**Exit Criteria**: 30-step curve renders in <45s; tooltips show portfolio details; click loads portfolio for analysis.

---

### Phase 6: SLM Integration (Week 7–8)

**Objective**: Local language model inference for portfolio explanations and back-test summaries.

| # | Task | Deliverable |
|---|------|-------------|
| 6.1 | Integrate ONNX Runtime Node (or llama.cpp bindings) | Inference runs in main process |
| 6.2 | Implement model manager (download, integrity check, selection) | `model-manager.ts` |
| 6.3 | Design prompt templates (portfolio explanation, back-test summary, sensitivity trade-off) | `resources/prompts/` |
| 6.4 | Implement streaming inference with token-by-token IPC | Main → renderer streaming |
| 6.5 | Build ExplanationPanel with typewriter effect + cancel | `ExplanationPanel.tsx` |
| 6.6 | Implement template-based fallback when no model available | `fallback.ts` |
| 6.7 | Add model download UI in Settings | Size, progress, select active model |
| 6.8 | Performance test: <10s explanation, streaming begins <2s | Benchmark suite |

**Exit Criteria**: SLM generates coherent explanations locally; streaming UX is smooth; graceful degradation when model unavailable.

---

### Phase 7: Persistence & Polish (Week 8–9)

**Objective**: Saved portfolios, data staleness handling, auto-update, and production hardening.

| # | Task | Deliverable |
|---|------|-------------|
| 7.1 | Implement portfolio persistence (save, list, rename, delete) | lowdb CRUD in portfolios.json + `SavedPage.tsx` |
| 7.2 | Persist back-test results linked to portfolios | Avoid re-computation |
| 7.3 | Implement staleness detection + refresh prompts | 30-day badge + refresh action |
| 7.4 | Configure electron-updater (auto-update) | Update flow in production builds |
| 7.5 | Implement offline mode (cached data, disabled live features) | Graceful degradation UX |
| 7.6 | Error handling audit (all unhandled → actionable messages) | Zero crash states |
| 7.7 | Memory profiling & optimization (<1GB budget) | Profiling report |
| 7.8 | Accessibility pass (keyboard nav, screen reader, contrast) | WCAG 2.1 AA basics |
| 7.9 | System tray integration (minimize to tray, background data refresh notifications) | Tray icon functional on all platforms |
| 7.10 | Final E2E test suite (all critical flows) | Green Playwright suite |

**Exit Criteria**: App persists all user data across sessions; handles offline gracefully; memory within budget; no unhandled errors; auto-update functional.

---

## Dependency Graph

```
Phase 0 (Foundation)
    │
    ▼
Phase 1 (Data Layer)
    │
    ├──────────────────────┐
    ▼                      ▼
Phase 2 (Optimizer)    Phase 6 (SLM) ← can start model integration in parallel
    │
    ├──────────────┐
    ▼              ▼
Phase 3          Phase 5
(Back-Test)      (Sensitivity)
    │              │
    ▼              │
Phase 4 ◄─────────┘
(Import/Rebalance)
    │
    ▼
Phase 7 (Persistence & Polish)
```

**Critical Path**: Phase 0 → 1 → 2 → 3 → 4 → 7  
**Parallel Track**: Phase 6 (SLM) can begin after Phase 1, integrates during Phase 7.  
**Phase 5** (Sensitivity) depends only on Phase 2 and can run in parallel with Phase 3.

---

## Risk Register

| Risk | Impact | Likelihood | Mitigation |
|------|--------|------------|------------|
| Free-tier API rate limits block development | High | Medium | Aggressive caching; mock data for dev; multiple provider fallback |
| Optimization solver performance on 500+ assets | Medium | Medium | Pre-filter universe by yield proximity; benchmark early in Phase 2 |
| SLM model size exceeds target hardware RAM | High | Low | Curate 1B–3B quantized models only; test on 8GB machines |
| Electron bundle size bloat from native modules | Medium | Medium | Tree-shake; lazy-load ONNX only when SLM requested |
| Historical data inconsistencies (splits, delistings) | Medium | High | Use only adjusted prices; validate against known benchmarks |
| Beta calculation sensitivity to lookback period | Low | Medium | Make lookback configurable; default to 2yr weekly (industry standard) |

---

## Complexity Tracking

> No constitution violations detected. Architecture follows all seven principles.

| Decision | Rationale | Simpler Alternative Considered |
|----------|-----------|-------------------------------|
| Separate optimization engine in main process | Avoids blocking renderer; leverages Node.js numeric libraries | Web Worker in renderer — rejected due to IPC complexity with file I/O |
| File-based JSON (lowdb + fs.promises) | Human-readable, zero native modules, sub-ms reads via in-memory arrays, atomic writes | SQLite — rejected to eliminate native module dependency and electron-rebuild maintenance |
| ONNX Runtime over pure llama.cpp | Better cross-platform support, easier model management | llama.cpp — kept as fallback option if ONNX model availability is limited |
