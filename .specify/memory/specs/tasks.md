# Tasks: Portfolio Recommender

**Input**: Design documents from `.specify/memory/specs/`
**Prerequisites**: `plan.md`, `portfolio-recommender.md`, `research.md`, `data-model.md`, `contracts/`
**Tests**: Mandatory. Per constitution, write tests first and verify they fail before implementation.
**Organization**: Tasks are grouped by plan phase and user story so each story can be implemented and validated independently.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Parallel-safe task (different files, no blocking dependency)
- **[Story]**: `US1`-`US6` for feature work, `INFRA` for shared infrastructure and cross-cutting tasks
- Include exact repository-relative file paths in every task description

## Phase 0: Foundation & Scaffold (Week 1)

**Purpose**: Bootable Electron app, working toolchain, typed IPC bridge, file-based storage bootstrap, and green CI pipeline.

### Tests for Phase 0 

> Write these tests first and confirm they fail before scaffolding the implementation.

- [ ] T001 [P] [INFRA] Add a failing first-launch smoke test in `tests/e2e/app-shell.e2e.ts` covering Electron window creation and renderer shell load.
- [ ] T002 [P] [INFRA] Add a failing preload bridge integration test in `tests/integration/ipc/preload-bridge.integration.test.ts` covering `contextBridge` exposure and safe IPC invocation.

### Implementation for Phase 0

- [ ] T003 [INFRA] Initialize the Electron + Vite + React + TypeScript workspace in `package.json`, `vite.config.ts`, `tsconfig.json`, `tsconfig.node.json`, and `src/renderer/index.html`.
- [ ] T004 [P] [INFRA] Configure linting, formatting, and styling in `.eslintrc.cjs`, `.prettierrc`, `tailwind.config.ts`, `postcss.config.cjs`, and `src/renderer/styles/globals.css`.
- [ ] T005 [P] [INFRA] Configure unit and E2E runners in `vitest.config.ts`, `playwright.config.ts`, and `package.json` so `npm run test` exercises the baseline suites.
- [ ] T006 [INFRA] Scaffold the split-process app shell in `src/main/index.ts`, `src/preload/index.ts`, `src/renderer/main.tsx`, `src/renderer/App.tsx`, and `src/shared/types/ipc.types.ts`.
- [ ] T007 [INFRA] Implement the initial typed IPC bridge in `src/preload/index.ts`, `src/shared/types/ipc.types.ts`, and `src/renderer/hooks/useIpc.ts`.
- [ ] T008 [P] [INFRA] Set up file-based storage layer: electron-store for preferences in `src/main/services/data/preferences.ts`, lowdb collections in `src/main/services/data/store.ts`, and atomic blob store in `src/main/services/data/blob-store.ts`. Create userData directory structure on first launch.
- [ ] T009 [P] [INFRA] Configure packaging outputs in `electron-builder.yml` and update build scripts in `package.json`.
- [ ] T010 [P] [INFRA] Add the CI workflow in `.github/workflows/ci.yml` for lint, typecheck, `npm audit`, tests, build, and renderer bundle budget checks.
- [ ] T011 [INFRA] Enforce renderer CSP and HTTPS-only network guardrails in `src/renderer/index.html`, `src/main/index.ts`, and `src/shared/validation/schemas.ts`.

**Checkpoint**: Phase 0 is complete when `npm run dev` opens the Electron shell, the preload bridge is reachable from the renderer, and CI can build the empty application.

---

## Phase 1: Data Layer & API Integration (Week 2)

**Purpose**: Shared market-data foundation with validated providers, caching, secure storage, rate limiting, and first-run settings UX.

### Tests for Phase 1 

> Write these tests first and confirm they fail before implementing the providers and settings flow.

- [ ] T012 [P] [INFRA] Add failing provider contract tests in `tests/integration/providers/alpha-vantage.integration.test.ts`, `tests/integration/providers/fmp.integration.test.ts`, `tests/integration/providers/polygon.integration.test.ts`, and `tests/integration/providers/fred.integration.test.ts`.
- [ ] T013 [P] [INFRA] Add failing cache TTL and offline fallback tests in `tests/unit/data/cache.test.ts`.
- [ ] T014 [P] [INFRA] Add failing secure-storage and provider rate-limiter tests in `tests/unit/data/secure-storage.test.ts` and `tests/unit/data/rate-limiter.test.ts`.
- [ ] T015 [P] [INFRA] Add failing universe seeding and first-run settings flow tests in `tests/integration/db/universe-seed.integration.test.ts` and `tests/e2e/settings-first-run.e2e.ts`.

### Implementation for Phase 1

- [ ] T016 [INFRA] Define shared market-data and settings contracts in `src/shared/types/asset.types.ts`, `src/shared/types/settings.types.ts`, and `src/shared/validation/schemas.ts`.
- [ ] T017 [INFRA] Implement the provider abstraction and HTTPS configuration validation in `src/main/services/data/providers/provider.interface.ts` and `src/main/services/data/providers/provider-config.ts`.
- [ ] T018 [P] [INFRA] Implement the Alpha Vantage adapter (primary — fundamentals, daily prices, Beta) with Zod response validation in `src/main/services/data/providers/alpha-vantage.provider.ts`.
- [ ] T019 [P] [INFRA] Implement the Financial Modeling Prep fallback adapter with Zod response validation in `src/main/services/data/providers/fmp.provider.ts`.
- [ ] T019b [P] [INFRA] Implement the Polygon.io bulk history provider (grouped daily bars for backfill) in `src/main/services/data/providers/polygon.provider.ts`.
- [ ] T019c [P] [INFRA] Implement the FRED provider (10Y Treasury risk-free rate) in `src/main/services/data/providers/fred.provider.ts`.
- [ ] T020 [INFRA] Implement TTL caching and stale-read behavior in `src/main/services/data/cache.ts` using per-response JSON files in userData/cache/ with atomic writes via `src/main/services/data/blob-store.ts`.
- [ ] T021 [P] [INFRA] Implement OS-native credential storage in `src/main/services/secure-storage.ts`.
- [ ] T022 [P] [INFRA] Implement token-bucket throttling in `src/main/services/data/rate-limiter.ts`.
- [ ] T023 [INFRA] Implement the universe seed and refresh flow in `scripts/seed-universe.ts` and `src/main/services/data/universe.repository.ts`.
- [ ] T024 [INFRA] Wire settings IPC access in `src/main/ipc/settings.ipc.ts`, `src/preload/index.ts`, and `src/shared/types/ipc.types.ts`.
- [ ] T025 [INFRA] Build the Settings page and API key wizard in `src/renderer/pages/SettingsPage.tsx`, `src/renderer/components/settings/ApiKeyWizard.tsx`, and `src/renderer/stores/settings.store.ts`.

**Checkpoint**: Phase 1 is complete when the app can fetch validated market data, cache it locally, store credentials securely, and complete the first-run setup flow.

---

## Phase 2: User Story 1 - Yield-Targeted Portfolio Generation (Priority: P1)  MVP

**Goal**: Let the user enter a target yield and receive an optimized portfolio of up to five assets with minimized aggregate Beta.

**Independent Test**: Enter a yield target on `GeneratePage`, run generation, and verify the returned portfolio meets the target, contains 1-5 assets, and reports weights, yields, Betas, and aggregate Beta.

### Tests for User Story 1 

> Write these tests first and confirm they fail before implementing the optimizer and generation UI.

- [ ] T026 [P] [US1] Add failing constraint and portfolio contract tests in `tests/unit/optimization/constraints.test.ts` and `tests/unit/optimization/portfolio.types.test.ts`.
- [ ] T027 [P] [US1] Add failing solver accuracy tests with known-answer fixtures in `tests/unit/optimization/optimizer.test.ts`.
- [ ] T028 [P] [US1] Add failing universe-filter and dropped-asset notification tests in `tests/unit/optimization/universe.test.ts` and `tests/integration/ipc/portfolio.ipc.integration.test.ts`.
- [ ] T029 [P] [US1] Add the failing end-to-end generation journey in `tests/e2e/generate-portfolio.e2e.ts`.

### Implementation for User Story 1

- [ ] T030 [US1] Define portfolio generation payloads and result types in `src/shared/types/portfolio.types.ts` and `src/shared/types/ipc.types.ts`.
- [ ] T031 [US1] Implement the yield, beta, weight-sum, and asset-count constraint builder in `src/main/services/optimization/constraints.ts`.
- [ ] T032 [US1] Implement the beta-minimizing solver and normalization helpers in `src/main/services/optimization/optimizer.ts` and `src/main/utils/math.ts`.
- [ ] T033 [US1] Implement freshness and history filters for the candidate universe in `src/main/services/optimization/universe.ts`.
- [ ] T034 [US1] Wire portfolio generation IPC handlers and dropped-asset warnings in `src/main/ipc/portfolio.ipc.ts` and `src/preload/index.ts`.
- [ ] T035 [US1] Build the generation workflow in `src/renderer/pages/GeneratePage.tsx` and `src/renderer/stores/portfolio.store.ts`.
- [ ] T036 [P] [US1] Build portfolio result presentation in `src/renderer/components/portfolio/PortfolioCard.tsx` and `src/renderer/components/portfolio/AssetTable.tsx`.
- [ ] T037 [US1] Implement impossible-yield, single-asset, and high-beta warning UX in `src/renderer/components/common/StatusAlert.tsx` and `src/renderer/pages/GeneratePage.tsx`.

**Checkpoint**: User Story 1 is complete when portfolio generation works end to end in under 10 seconds and all optimizer edge-case tests pass.

---

## Phase 3: User Story 2 - Historical Back-Testing (Priority: P1)

**Goal**: Let the user back-test a generated portfolio across a selected historical date range with metrics, progress, and interactive charts.

**Independent Test**: Run a five-year back-test for a generated portfolio and verify cumulative return, annualized return, volatility, drawdown, Sharpe, Sortino, and yield-on-cost outputs render with progress feedback.

### Tests for User Story 2 ⚠

> Write these tests first and confirm they fail before implementing the back-test engine and charts.

- [ ] T038 [P] [US2] Add failing daily replay and dividend reinvestment tests in `tests/unit/backtest/engine.test.ts`.
- [ ] T039 [P] [US2] Add failing Sharpe, Sortino, drawdown, and annualization tests in `tests/unit/backtest/metrics.test.ts`.
- [ ] T040 [P] [US2] Add failing rebalance-schedule and missing-history tests in `tests/unit/backtest/rebalancer.test.ts` and `tests/unit/backtest/missing-data.test.ts`.
- [ ] T041 [P] [US2] Add failing IPC progress, SPY benchmark, and end-to-end flow tests in `tests/integration/ipc/backtest.ipc.integration.test.ts`, `tests/integration/backtest/benchmark.integration.test.ts`, and `tests/e2e/backtest-flow.e2e.ts`.

### Implementation for User Story 2

- [ ] T042 [US2] Define back-test request, progress, and result contracts in `src/shared/types/backtest.types.ts` and `src/shared/types/ipc.types.ts`.
- [ ] T043 [US2] Implement the adjusted-price historical replay engine with dividend reinvestment in `src/main/services/backtest/engine.ts`.
- [ ] T044 [P] [US2] Implement risk and performance metric calculations in `src/main/services/backtest/metrics.ts`.
- [ ] T045 [P] [US2] Implement monthly, quarterly, annual, and never rebalancing schedules in `src/main/services/backtest/rebalancer.ts`.
- [ ] T046 [US2] Wire progress-aware back-test IPC in `src/main/ipc/backtest.ipc.ts`, `src/preload/index.ts`, and `src/renderer/stores/backtest.store.ts`.
- [ ] T047 [P] [US2] Build the interactive charts in `src/renderer/components/charts/EquityCurve.tsx`, `src/renderer/components/charts/DrawdownChart.tsx`, and `src/renderer/components/charts/YieldTimeChart.tsx`.
- [ ] T048 [US2] Build the back-test configuration and summary dashboard in `src/renderer/pages/BackTestPage.tsx` and `src/renderer/components/common/ProgressBar.tsx`.
- [ ] T049 [US2] Handle pre-IPO, delisting, and split-adjustment edge cases in `src/main/services/backtest/engine.ts` and `src/main/services/backtest/metrics.ts`.

**Checkpoint**: User Story 2 is complete when back-tests finish within the performance budget, the UI stays responsive with progress updates, and benchmark validation stays within 0.5%.

---

## Phase 4: User Story 3 - Portfolio Import & Rebalance (Priority: P2)

**Goal**: Let the user import an existing portfolio, validate holdings, generate a lower-Beta rebalance, and compare original versus rebalanced performance.

**Independent Test**: Import a CSV or manually enter holdings, rebalance to a target yield, and confirm the app shows the imported portfolio, the rebalanced portfolio, and side-by-side comparison results.

### Tests for User Story 3 

> Write these tests first and confirm they fail before implementing import, validation, and comparison workflows.

- [ ] T050 [P] [US3] Add failing CSV import format tests in `tests/unit/import/csv-parser.test.ts`.
- [ ] T051 [P] [US3] Add failing ticker validation and fuzzy-match tests in `tests/unit/import/validator.test.ts`.
- [ ] T052 [P] [US3] Add failing import-to-rebalance integration and comparison journey tests in `tests/integration/import/rebalance.integration.test.ts` and `tests/e2e/import-rebalance.e2e.ts`.

### Implementation for User Story 3

- [ ] T053 [US3] Implement CSV normalization for `ticker,shares` and `ticker,weight` formats in `src/main/services/import/csv-parser.ts`.
- [ ] T054 [US3] Implement supported-universe lookup and Levenshtein suggestions in `src/main/services/import/validator.ts`.
- [ ] T055 [US3] Define import, validation, and rebalance payloads in `src/shared/types/import.types.ts` and `src/shared/types/ipc.types.ts`.
- [ ] T056 [US3] Wire import, validate, and rebalance handlers in `src/main/ipc/import.ipc.ts` and `src/preload/index.ts`.
- [ ] T057 [US3] Build the upload, validation, and correction flow in `src/renderer/components/portfolio/ImportWizard.tsx` and `src/renderer/pages/ImportPage.tsx`.
- [ ] T058 [P] [US3] Build manual holdings entry with supported-symbol autocomplete in `src/renderer/components/portfolio/ManualHoldingsForm.tsx`.
- [ ] T059 [US3] Build original-vs-rebalanced comparison views and overlaid charts in `src/renderer/components/portfolio/ComparisonView.tsx` and `src/renderer/pages/ImportPage.tsx`.
- [ ] T060 [US3] Implement CSV and PDF export actions in `src/main/services/export/portfolio-export.ts` and `src/main/ipc/import.ipc.ts`.

**Checkpoint**: User Story 3 is complete when a 50-holding import can be validated, rebalanced, and compared end to end in under 5 seconds for the import stage.

---

## Phase 5: User Story 4 - Yield & Beta Sensitivity Curve (Priority: P2)

**Goal**: Let the user generate and interact with a Yield-vs-Beta frontier, inspect compositions at each point, and load a selected point back into portfolio generation.

**Independent Test**: Generate a 30-step curve for a chosen yield range, verify tooltips show portfolio composition, verify progress updates appear, and confirm clicking a point hydrates the Generate view.

### Tests for User Story 4 

> Write these tests first and confirm they fail before implementing the sensitivity engine and chart.

- [ ] T061 [P] [US4] Add failing yield-sweep generator tests in `tests/unit/sensitivity/curve-generator.test.ts`.
- [ ] T062 [P] [US4] Add failing sensitivity IPC progress and interaction tests in `tests/integration/ipc/sensitivity.ipc.integration.test.ts` and `tests/e2e/sensitivity-curve.e2e.ts`.

### Implementation for User Story 4

- [ ] T063 [US4] Define sensitivity request, point, and result contracts in `src/shared/types/sensitivity.types.ts` and `src/shared/types/ipc.types.ts`.
- [ ] T064 [US4] Implement iterative curve generation across target-yield ranges in `src/main/services/sensitivity/curve-generator.ts`.
- [ ] T065 [US4] Optimize the curve sweep with infeasible-target early exit and shared math helpers in `src/main/services/sensitivity/curve-generator.ts` and `src/main/utils/math.ts`.
- [ ] T066 [US4] Wire progress-aware sensitivity IPC in `src/main/ipc/sensitivity.ipc.ts`, `src/preload/index.ts`, and `src/renderer/stores/sensitivity.store.ts`.
- [ ] T067 [US4] Build the interactive frontier chart with composition tooltips in `src/renderer/components/charts/SensitivityChart.tsx` and `src/renderer/pages/SensitivityPage.tsx`.
- [ ] T068 [US4] Implement click-to-load hydration into portfolio generation state in `src/renderer/pages/SensitivityPage.tsx` and `src/renderer/stores/portfolio.store.ts`.
- [ ] T069 [US4] Plot the imported-portfolio marker and comparison legend in `src/renderer/components/charts/SensitivityChart.tsx` and `src/renderer/pages/SensitivityPage.tsx`.

**Checkpoint**: User Story 4 is complete when a 30-step curve renders within 45 seconds, exposes per-point portfolio details, and can launch further analysis from any selected point.

---

## Phase 6: User Story 5 - SLM-Powered Insights & Explanations (Priority: P3)

**Goal**: Let the user request local-model explanations for generated portfolios, back-tests, and sensitivity trade-offs with streaming output and graceful fallback.

**Independent Test**: Trigger an explanation after generating a portfolio or back-test, verify streamed text appears with cancel support, and verify the fallback template is offered when no valid model is installed.

### Tests for User Story 5 

> Write these tests first and confirm they fail before implementing local inference, model management, and streaming UI.

- [ ] T070 [P] [US5] Add failing inference and fallback unit tests in `tests/unit/slm/inference.test.ts` and `tests/unit/slm/fallback.test.ts`.
- [ ] T071 [P] [US5] Add failing model-manager and streaming IPC tests in `tests/unit/slm/model-manager.test.ts` and `tests/integration/ipc/slm.ipc.integration.test.ts`.
- [ ] T072 [P] [US5] Add failing explanation workflow and latency-budget tests in `tests/e2e/slm-explanation.e2e.ts` and `tests/integration/slm/performance.integration.test.ts`.

### Implementation for User Story 5

- [ ] T073 [US5] Add prompt templates for portfolio, back-test, and sensitivity narratives in `resources/prompts/explain-portfolio.txt`, `resources/prompts/summarize-backtest.txt`, and `resources/prompts/explain-sensitivity.txt`.
- [ ] T074 [US5] Implement the local inference runner with cancellation support in `src/main/services/slm/inference.ts`.
- [ ] T075 [US5] Implement model download, integrity validation, and active-model selection in `src/main/services/slm/model-manager.ts` and `scripts/download-model.ts`.
- [ ] T076 [US5] Implement prompt assembly and template fallback generation in `src/main/services/slm/prompts.ts` and `src/main/services/slm/fallback.ts`.
- [ ] T077 [US5] Wire streaming SLM IPC channels in `src/main/ipc/slm.ipc.ts`, `src/preload/index.ts`, and `src/shared/types/ipc.types.ts`.
- [ ] T078 [US5] Build the streaming explanation UX in `src/renderer/components/slm/ExplanationPanel.tsx`, `src/renderer/components/slm/StreamingText.tsx`, and `src/renderer/hooks/useStreamingSlm.ts`.
- [ ] T079 [US5] Add model download and selection controls in `src/renderer/components/settings/ModelDownloadPanel.tsx`, `src/renderer/pages/SettingsPage.tsx`, and `src/renderer/stores/settings.store.ts`.
- [ ] T080 [US5] Add fallback disclaimer and report-issue logging in `src/renderer/components/slm/ExplanationPanel.tsx` and `src/main/utils/logger.ts`.

**Checkpoint**: User Story 5 is complete when explanations stream smoothly from a local model, begin within the latency budget, and gracefully fall back when the model is missing or corrupted.

---

## Phase 7: User Story 6 - Persistent Portfolio Management & Polish (Week 8-9, Priority: P3)

**Goal**: Persist portfolios and back-tests across sessions while hardening the app for offline use, updates, accessibility, memory limits, and final release quality.

**Independent Test**: Save a generated portfolio and back-test, restart the app, verify the saved data is still available, then exercise offline mode, stale-data refresh prompts, and final regression coverage.

### Tests for User Story 6 and Polish 

> Write these tests first and confirm they fail before implementing persistence, offline behavior, and release hardening.

- [ ] T081 [P] [US6] Add failing persistence CRUD and restart-flow tests in `tests/integration/db/persistence.integration.test.ts` and `tests/e2e/saved-portfolios.e2e.ts`.
- [ ] T082 [P] [US6] Add failing stale-data, offline-mode, and rename/delete tests in `tests/unit/data/staleness.test.ts`, `tests/integration/ipc/offline-mode.integration.test.ts`, and `tests/e2e/offline-mode.e2e.ts`.
- [ ] T083 [P] [INFRA] Add failing accessibility and tray/update smoke tests in `tests/e2e/accessibility.e2e.ts` and `tests/integration/shell/tray-update.integration.test.ts`.

### Implementation for User Story 6 and Polish

- [ ] T084 [US6] Implement portfolio and back-test repositories in `src/main/services/data/repositories/portfolio.repository.ts` (lowdb CRUD on portfolios.json) and `src/main/services/data/repositories/backtest.repository.ts` (atomic read/write per backtest in userData/backtests/).
- [ ] T085 [US6] Wire save, list, rename, delete, and persisted back-test IPC flows in `src/main/ipc/portfolio.ipc.ts`, `src/main/ipc/backtest.ipc.ts`, and `src/preload/index.ts`.
- [ ] T086 [US6] Build the saved-portfolio list and detail view in `src/renderer/pages/SavedPage.tsx` and `src/renderer/components/portfolio/SavedPortfolioList.tsx`.
- [ ] T087 [US6] Implement stale-data badges and refresh actions in `src/renderer/components/common/StaleBadge.tsx`, `src/renderer/pages/SavedPage.tsx`, and `src/renderer/stores/portfolio.store.ts`.
- [ ] T088 [US6] Implement offline-mode capability gating and cached-data messaging in `src/main/services/data/cache.ts`, `src/renderer/components/common/OfflineBanner.tsx`, and `src/renderer/pages/SettingsPage.tsx`.
- [ ] T089 [P] [INFRA] Configure auto-update behavior in `src/main/index.ts`, `electron-builder.yml`, and `package.json`.
- [ ] T090 [INFRA] Complete the error-state audit across main and renderer in `src/main/utils/logger.ts`, `src/renderer/components/common/ErrorBoundary.tsx`, `src/renderer/pages/GeneratePage.tsx`, and `src/renderer/pages/BackTestPage.tsx`.
- [ ] T091 [P] [INFRA] Add memory profiling and budget assertions in `scripts/profile-memory.ts` and `tests/integration/performance/memory.integration.test.ts`.
- [ ] T092 [INFRA] Apply keyboard navigation, screen-reader labels, and contrast fixes in `src/renderer/App.tsx`, `src/renderer/pages/GeneratePage.tsx`, `src/renderer/pages/ImportPage.tsx`, `src/renderer/pages/SavedPage.tsx`, and `src/renderer/pages/SettingsPage.tsx`.
- [ ] T093 [P] [INFRA] Implement system tray behavior and background refresh notifications in `src/main/tray.ts` and `src/main/index.ts`.
- [ ] T094 [INFRA] Finalize regression coverage in `tests/e2e/generate-portfolio.e2e.ts`, `tests/e2e/backtest-flow.e2e.ts`, `tests/e2e/import-rebalance.e2e.ts`, `tests/e2e/sensitivity-curve.e2e.ts`, `tests/e2e/slm-explanation.e2e.ts`, and `tests/e2e/saved-portfolios.e2e.ts`.

**Checkpoint**: User Story 6 and release polish are complete when saved data survives restart, stale and offline states are actionable, update/tray flows work, memory stays within budget, and the final E2E suite is green.

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 0 -> Phase 1**: Complete the shell, IPC bridge, file-based storage bootstrap, and CI before implementing market-data services.
- **Phase 1 -> Phase 2**: Provider contracts, cache, secure storage, and seeded universe are required before optimizer work can start.
- **Phase 2 -> Phase 3**: Back-testing depends on generated portfolio contracts and shared market-history access.
- **Phase 2 -> Phase 5**: Sensitivity analysis depends on the optimizer and can start once User Story 1 is stable.
- **Phase 3 -> Phase 4**: Import/rebalance comparisons depend on both optimization and back-testing flows.
- **Phase 1 -> Phase 6**: SLM integration can begin after shared data contracts and settings infrastructure exist, then integrate with later stories as they land.
- **Phase 2/3/4/5/6 -> Phase 7**: Persistence and polish should start after the core feature payloads are stable enough to persist and regression-test.

### User Story Dependencies

- **US1 (P1)**: Starts immediately after Phase 1 and delivers the MVP.
- **US2 (P1)**: Depends on US1 portfolio outputs and shared historical data access.
- **US3 (P2)**: Depends on US1 optimization plus US2 comparative back-test infrastructure.
- **US4 (P2)**: Depends on US1 optimizer only and can run in parallel with US2 once Phase 2 is complete.
- **US5 (P3)**: Depends on Phase 1 settings/data scaffolding and consumes outputs from US1, US2, and US4.
- **US6 (P3)**: Depends on stable contracts from US1-US4 and should absorb US5 persistence hooks before final release hardening.

### Within Each User Story

- Tests MUST be written and observed failing before implementation tasks begin.
- Shared type and schema updates should land before IPC handlers and renderer wiring.
- Main-process services should land before preload exposure and UI consumption.
- UI state stores should be wired before final E2E validation for that story.
- Finish the story checkpoint before moving to the next priority unless tasks are explicitly marked `[P]` and touch different files.

### Parallel Opportunities

- All tasks marked `[P]` are safe to split across team members because they target different files or isolated test surfaces.
- In Phase 1, `alpha-vantage.provider.ts`, `fmp.provider.ts`, `polygon.provider.ts`, `fred.provider.ts`, `secure-storage.ts`, and `rate-limiter.ts` can be implemented in parallel after `provider.interface.ts` lands.
- In US1, result components (`PortfolioCard.tsx`, `AssetTable.tsx`) can proceed in parallel with solver work once portfolio contracts are defined.
- In US2, chart components can proceed in parallel with `metrics.ts` and `rebalancer.ts` after back-test result contracts are defined.
- In US3, manual entry UI can proceed in parallel with CSV parsing once import payload types exist.
- In Phase 7, auto-update, memory profiling, and tray integration can proceed in parallel with persistence UI once the saved-portfolio contracts are stable.

### Recommended Delivery Order

1. Finish **Phase 0** and **Phase 1** to establish the shared shell and data foundation.
2. Deliver **US1** as the MVP and validate it independently.
3. Deliver **US2** and **US4** next; US4 can overlap with US2 once the optimizer is stable.
4. Deliver **US3** after US2 back-test infrastructure is usable.
5. Deliver **US5** once the core quantitative outputs are stable enough to explain.
6. Deliver **US6** and the remaining polish tasks last, then run the full regression suite.
