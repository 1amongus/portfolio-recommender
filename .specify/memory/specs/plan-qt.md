## Summary

Portfolio Recommender will be rewritten as a cross-platform desktop application using C++17, Qt 6.7+, QML, and Qt Widgets/Qt Charts to deliver a fast native experience on Windows, macOS, and Linux. The application will let users target a dividend yield, generate an optimized portfolio of up to five stocks or ETFs using beta minimization, back-test the resulting allocation historically, import and rebalance existing holdings, inspect yield/beta sensitivity curves, and receive natural-language explanations generated locally by small language models running through ONNX Runtime.

The rewrite emphasizes a clear separation between UI, domain models, business services, data providers, and persistence. Qt Quick/QML will handle the modern navigational shell and page composition, while C++ services will own optimization, back-testing, data orchestration, storage, and inference. The architecture will prioritize deterministic calculations, responsive asynchronous networking, resilient caching, atomic file persistence, and testability at the service boundary.

Success for this plan means shipping a maintainable desktop product that:
- Produces portfolio recommendations consistent with the target yield and <=5-asset constraint.
- Measures risk using beta-aware optimization and rich historical metrics.
- Supports portfolio import and rebalance workflows for real-world use.
- Remains responsive during network, optimization, back-test, and inference operations.
- Works consistently across Windows, macOS, and Linux with predictable packaging and upgrade paths.

## Technical Context

### Core Stack
- **Language:** C++17
- **Build system:** CMake 3.21+
- **UI Framework:** Qt 6.7+ with QML/Qt Quick for application shell and user flows, plus Qt Widgets where richer chart container behavior or desktop-specific interaction is easier to implement.
- **Charts:** Qt Charts module for equity curves, drawdown views, yield history, and sensitivity plots.
- **Network:** QNetworkAccessManager with fully asynchronous request/response flow, retry handling, and provider failover.
- **Storage:** QSettings for user preferences and lightweight settings; QJsonDocument + QFile for domain data persistence using an atomic temp-file-then-rename write pattern.
- **SLM Runtime:** ONNX Runtime C++ API for local inference and explanation generation.
- **Testing:** Qt Test + CTest for unit/integration coverage; Squish for end-to-end UI smoke/regression tests.
- **Packaging:** CPack for build artifacts plus Qt Installer Framework for production installers.
- **Platforms:** Windows 10+, macOS 12+, Linux (Ubuntu 22.04+).

### Architectural Principles
1. **Native responsiveness first.** All network, optimization, back-test, and SLM work must be asynchronous or offloaded so the UI thread stays responsive.
2. **Thin UI, rich services.** QML pages and controllers should orchestrate state and user interaction, while business rules live in testable C++ services.
3. **Deterministic domain logic.** Optimization, back-test, and metric calculations should be pure or near-pure where possible to simplify testing and reproducibility.
4. **Provider abstraction and fallback.** External APIs must sit behind provider contracts so fallback routing, caching, and rate limiting are centrally managed.
5. **File-based durability.** User data, cached responses, saved portfolios, and back-test artifacts should be stored locally without requiring a database.
6. **Cross-platform from day one.** Paths, packaging, settings storage, and deployment assumptions should be implemented with platform abstraction instead of platform-specific rewrites later.

### Non-Functional Requirements
- UI navigation should feel instantaneous, with page transitions under 100 ms in normal conditions.
- Long-running operations should surface progress, cancellation, and failure states.
- Portfolio generation and rebalance results should be reproducible from the same inputs and market snapshot.
- Cached market data should minimize repeated API usage and make the app usable under provider rate limits.
- Local storage writes must be crash-safe and resilient to partial write failures.
- The shipped desktop binary footprint should stay below the bundle size target where feasible, excluding optional model downloads.

### External Integrations
- **Alpha Vantage:** Primary market/fundamental data source for quotes, dividend history, and overview data.
- **FMP:** Secondary/fallback source when Alpha Vantage data is incomplete or throttled.
- **Polygon.io:** Bulk historical price/dividend data for efficient back-testing.
- **FRED:** Macro rate inputs used for risk-free rate assumptions and contextual metrics.
- **ONNX models:** Local SLM assets for explainability, downloaded or discovered through model management.

## Project Structure

```text
portfolio-recommender/
|-- CMakeLists.txt                    # Root CMake
|-- cmake/                            # CMake modules (FindOnnxRuntime, etc.)
|-- src/
|   |-- app/                          # Application entry, main window
|   |   |-- main.cpp
|   |   `-- Application.cpp/h
|   |-- models/                       # Data models (C++ classes)
|   |   |-- Portfolio.cpp/h
|   |   |-- Asset.cpp/h
|   |   |-- BacktestResult.cpp/h
|   |   `-- SensitivityPoint.cpp/h
|   |-- services/                     # Business logic
|   |   |-- optimization/
|   |   |   |-- Optimizer.cpp/h
|   |   |   |-- Constraints.cpp/h
|   |   |   `-- Universe.cpp/h
|   |   |-- backtest/
|   |   |   |-- Engine.cpp/h
|   |   |   |-- Metrics.cpp/h
|   |   |   `-- Rebalancer.cpp/h
|   |   |-- sensitivity/
|   |   |   `-- CurveGenerator.cpp/h
|   |   |-- data/
|   |   |   |-- providers/
|   |   |   |   |-- IMarketDataProvider.h    # Interface
|   |   |   |   |-- AlphaVantageProvider.cpp/h
|   |   |   |   |-- FmpProvider.cpp/h
|   |   |   |   |-- PolygonProvider.cpp/h
|   |   |   |   `-- FredProvider.cpp/h
|   |   |   |-- DataStore.cpp/h              # JSON file storage
|   |   |   |-- RateLimiter.cpp/h
|   |   |   `-- Cache.cpp/h
|   |   `-- slm/
|   |       |-- InferenceEngine.cpp/h
|   |       `-- ModelManager.cpp/h
|   |-- ui/                           # QML + Qt Quick
|   |   |-- qml/
|   |   |   |-- Main.qml
|   |   |   |-- pages/
|   |   |   |   |-- GeneratePage.qml
|   |   |   |   |-- BackTestPage.qml
|   |   |   |   |-- SensitivityPage.qml
|   |   |   |   |-- ImportPage.qml
|   |   |   |   |-- SavedPage.qml
|   |   |   |   `-- SettingsPage.qml
|   |   |   `-- components/
|   |   |       |-- NavigationBar.qml
|   |   |       |-- PortfolioCard.qml
|   |   |       |-- YieldChart.qml
|   |   |       `-- LoadingOverlay.qml
|   |   `-- controllers/              # C++ QML-exposed controllers
|   |       |-- PortfolioController.cpp/h
|   |       |-- BacktestController.cpp/h
|   |       |-- SensitivityController.cpp/h
|   |       |-- ImportController.cpp/h
|   |       `-- SettingsController.cpp/h
|   |-- utils/
|   |   |-- Math.cpp/h                # Financial math (Sharpe, Sortino, Beta, etc.)
|   |   |-- Logger.cpp/h
|   |   `-- FileUtils.cpp/h           # Atomic file operations
|   `-- resources/
|       |-- qml.qrc
|       `-- icons/
|-- tests/
|   |-- unit/
|   |   |-- tst_optimizer.cpp
|   |   |-- tst_metrics.cpp
|   |   |-- tst_datastore.cpp
|   |   `-- tst_ratelimiter.cpp
|   `-- integration/
|       `-- tst_provider_contract.cpp
|-- packaging/
|   |-- windows/                      # NSIS/WiX installer config
|   |-- macos/                        # .app bundle, DMG config
|   `-- linux/                        # AppImage/Flatpak config
`-- docs/
    `-- architecture.md
```

### Structure Notes
- `src/models` contains serializable domain objects with minimal behavior and clear JSON conversion boundaries.
- `src/services` contains computation-heavy and integration-heavy logic, isolated from QML so it is directly unit testable.
- `src/ui/controllers` exposes stable Qt properties, signals, and invokable methods to QML, translating UI state into service calls.
- `src/utils` holds shared helpers only when they are generic and cross-cutting; finance-specific logic remains in `services` or `models`.
- `tests/unit` focuses on deterministic calculations and persistence edge cases, while `tests/integration` validates provider contracts, cache behavior, and fallback flows.
- `packaging` keeps deployment assets out of runtime code and makes platform-specific installer evolution explicit.

## Phases

### Phase 0 - Project Scaffold & Build System (Week 1)

**Objectives**
- Establish a clean Qt/CMake foundation that can build and run on all supported platforms.
- Prove the application shell, navigation model, dependency wiring, and CI pipeline before business logic begins.

**Implementation Work**
- Create the root `CMakeLists.txt` with Qt module discovery, warning settings, target layout, and test registration.
- Add `cmake/` helpers for optional dependencies, compiler flags, and future ONNX Runtime discovery.
- Implement `main.cpp` and `Application` bootstrap code to initialize `QApplication`, QML engine, logging, and settings paths.
- Build the initial `Main.qml` shell with `StackView` or equivalent navigation across six primary pages.
- Create page stubs for Generate, Back-Test, Sensitivity, Import, Saved, and Settings views.
- Add GitHub Actions CI with formatting verification, configure/build steps, and `ctest` execution on a platform matrix or at minimum Linux plus one desktop OS.
- Document developer setup expectations in `docs/architecture.md` or repository bootstrap documentation.

**Key Deliverables**
- Successful local and CI builds.
- Launchable desktop shell with consistent page navigation.
- Basic logging, settings initialization, and resource loading.

**Exit Criteria**
- App starts reliably on all target developer platforms.
- CI validates formatting, compilation, and tests on each pull request.
- All future modules can be added without reworking the build layout.

### Phase 1 - Data Layer (Week 2)

**Objectives**
- Implement a resilient data acquisition and persistence layer that hides provider differences from the rest of the app.
- Establish caching, rate limiting, and settings management early to control external API cost and latency.

**Implementation Work**
- Define `IMarketDataProvider` contracts for quotes, historical prices, dividend series, security metadata, and macro rates.
- Implement `AlphaVantageProvider` as the initial production source with structured response parsing and explicit error surfaces.
- Add `RateLimiter` using a token bucket or leaky bucket strategy tuned per provider quota.
- Create `Cache` with SHA256-keyed entries, TTL metadata, and per-endpoint invalidation rules.
- Implement `DataStore` for saved portfolios, cached back-test snapshots, imported holdings, and application state using `QJsonDocument` plus atomic rename writes.
- Wire `QSettings` for API keys, theme choice, model paths, chart preferences, and default back-test assumptions.
- Add provider fallback orchestration so Alpha Vantage failure paths can route to FMP and bulk history requests can route to Polygon where appropriate.
- Write unit tests for parsing, persistence, rate limiting, cache hits/misses, and corrupted-file recovery.

**Key Deliverables**
- Stable provider interfaces and concrete Alpha Vantage integration.
- Reusable cache and persistence services.
- User preference storage and recovery.

**Exit Criteria**
- A controller or service can request market data without knowing which provider fulfilled it.
- Storage writes are atomic and verified with failure-mode tests.
- Provider errors are observable, recoverable, and do not freeze the UI.

### Phase 2 - Optimization Engine (Week 3)

**Objectives**
- Deliver the first end-to-end core value path: input target yield, evaluate the investment universe, and return an optimized <=5-asset portfolio.
- Make optimization logic deterministic, explainable, and test-driven.

**Implementation Work**
- Build `Universe` assembly logic that filters eligible assets based on data completeness, dividend availability, and configurable rules.
- Implement `Constraints` for target yield floor, max asset count, minimum/maximum weight, optional ETF/stock mix limits, and exclusion rules.
- Create the `Optimizer` using beta-minimizing selection with a greedy candidate pass followed by weight optimization/refinement.
- Centralize finance utilities such as beta estimation, expected yield aggregation, normalization, and sanity bounds in `utils/Math` or domain-specific helpers.
- Define result models that include holdings, weights, expected yield, portfolio beta, and diagnostic metadata explaining why candidates were chosen or rejected.
- Expose portfolio generation through `PortfolioController` with loading state, validation, and error reporting for QML.
- Implement `GeneratePage.qml` forms, validation messaging, and recommendation display cards.
- Drive implementation with Qt Test coverage for constraints, optimization edge cases, and reproducibility.

**Key Deliverables**
- Functional portfolio generator from target yield input to rendered recommendation.
- Core math library for downstream analytics.
- Test suite covering optimizer correctness and edge cases.

**Exit Criteria**
- The app can generate valid <=5-asset recommendations from realistic sample data.
- Results respect all configured constraints.
- Optimization output is stable across repeated runs with identical inputs.

### Phase 3 - Back-Test Engine (Week 4)

**Objectives**
- Validate generated or imported portfolios through historical simulation and expose investor-friendly performance metrics.
- Make historical analysis a first-class feature rather than a post-processing add-on.

**Implementation Work**
- Implement Polygon-backed historical price/dividend acquisition optimized for back-test windows and bulk retrieval.
- Build the `Engine` daily simulation loop with holdings state, cash handling, dividend accrual, optional periodic rebalance, and benchmark comparisons if desired.
- Implement `Metrics` for total return, CAGR, Sharpe ratio, Sortino ratio, max drawdown, rolling yield stability, volatility, and beta over time where applicable.
- Define `BacktestResult` and supporting time-series models for equity curve, drawdown curve, income curve, and summary panels.
- Expose back-test operations through `BacktestController` with progress updates and cancellation hooks.
- Implement `BackTestPage.qml` including parameter inputs, run-state feedback, and results presentation.
- Add Qt Charts views for equity curve and drawdown, with dataset downsampling for rendering performance.
- Create unit and integration tests validating metric formulas, dividend handling, and provider contract assumptions.

**Key Deliverables**
- End-to-end back-test workflow for generated and imported portfolios.
- Rich performance metrics and charts.
- Tested simulation engine with deterministic fixtures.

**Exit Criteria**
- Users can run a historical back-test without blocking the UI.
- Equity curve and summary metrics align with known fixture expectations.
- Large history pulls remain performant through caching and bulk retrieval.

### Phase 4 - Sensitivity Curve (Week 5)

**Objectives**
- Show how portfolio characteristics change across different target yields so users can understand the trade-off frontier.
- Reuse optimization logic systematically rather than building a one-off visualization path.

**Implementation Work**
- Implement `CurveGenerator` to sweep target yields across a configurable range and invoke the optimizer per step.
- Generate `SensitivityPoint` records containing yield target, achieved yield, portfolio beta, asset count, and feasibility markers.
- Add progress reporting signals and cancellation support because multi-point sweeps can be computationally heavy.
- Expose chart-ready data and summary insights through `SensitivityController`.
- Build `SensitivityPage.qml` with interactive plotting, hover tooltips, and selectable sweep parameters.
- Add visual indicators for infeasible ranges, unstable optimization zones, or areas where constraints dominate.
- Write tests for curve generation correctness, monotonic assumptions where valid, and failure behavior when insufficient data exists.

**Key Deliverables**
- Yield/Beta sensitivity visualization.
- Reusable sweep engine for advanced analysis.
- Responsive progress-aware UI.

**Exit Criteria**
- Users can generate and inspect a sensitivity curve from the desktop UI.
- Sweep results are reproducible and correctly reflect optimizer feasibility.
- The chart remains responsive even for dense result sets.

### Phase 5 - Import & Rebalance (Week 6)

**Objectives**
- Support real investor workflows by allowing existing holdings to be imported, analyzed, and improved rather than requiring greenfield portfolio creation.
- Extend optimization to operate under current-position constraints.

**Implementation Work**
- Implement CSV import via `QTextStream`, including delimiter detection, header mapping, ticker/weight/quantity parsing, and validation errors.
- Add manual-entry workflows with ticker autocomplete backed by cached universe metadata.
- Build normalized imported portfolio models with optional cost basis and account notes.
- Implement rebalance logic that uses optimization targets while respecting existing holdings, turnover limits, optional no-sell rules, and max-asset constraints.
- Create comparison models for before/after yield, beta, turnover, diversification, and projected income.
- Expose workflows through `ImportController` and render them in `ImportPage.qml`.
- Build a side-by-side comparison view with highlights for keep/add/reduce/remove actions.
- Write tests for CSV parsing, rebalance constraints, and invalid input recovery.

**Key Deliverables**
- Import pipeline for external portfolios.
- Rebalance recommendations grounded in current holdings.
- Comparison UI for actionable decision-making.

**Exit Criteria**
- A user can import a typical brokerage CSV or manually enter a portfolio.
- The app can recommend a rebalanced allocation under stated constraints.
- Validation errors are clear and prevent silent data corruption.

### Phase 6 - SLM Integration (Week 7)

**Objectives**
- Add offline natural-language explanations that help users understand recommendations, back-test outcomes, and rebalance trade-offs.
- Keep inference local, optional, and non-blocking.

**Implementation Work**
- Integrate ONNX Runtime C++ into the build and runtime packaging flow.
- Implement `ModelManager` for model discovery, version metadata, optional downloads, integrity verification, and storage location management.
- Build `InferenceEngine` to transform structured portfolio/back-test context into promptable inputs and parse generated explanations.
- Run inference in `QThread`, `QtConcurrent`, or a dedicated worker pool so the UI remains responsive.
- Define guardrails for prompt construction, token budget, timeout handling, and fallback behavior when no local model is installed.
- Surface explanation requests through the relevant controllers and render them in QML panels with loading/error states.
- Add tests for model availability handling, prompt assembly, and deterministic stubs or mocks around inference boundaries.

**Key Deliverables**
- Local explanation generation integrated into the recommendation workflow.
- Model lifecycle management.
- Non-blocking inference execution path.

**Exit Criteria**
- Users can request or automatically receive a local explanation for generated recommendations.
- Missing model scenarios degrade gracefully.
- Inference does not block UI navigation or long-running calculations.

### Phase 7 - Polish & Packaging (Week 8-9)

**Objectives**
- Prepare the application for production use across supported desktop platforms.
- Reduce operational risk through packaging, observability, and final performance work.

**Implementation Work**
- Refine UX details such as empty states, skeleton loaders, validation copy, keyboard navigation, and accessibility basics.
- Add system tray integration where it adds value for long-running syncs, notifications, or quick-open behavior.
- Implement or integrate an auto-update approach appropriate to each platform and installer strategy.
- Finalize CPack and Qt Installer Framework assets for Windows, macOS, and Linux targets.
- Add cross-platform packaging CI jobs, signing hooks where available, and artifact publishing.
- Profile build and runtime performance; add PCH, ccache, resource trimming, and chart downsampling where needed.
- Verify install/uninstall behavior, settings migration, cache directory cleanup, and offline startup behavior.
- Execute end-to-end regression coverage with Squish and targeted manual QA scripts.

**Key Deliverables**
- Production-quality installers.
- Stable UX and performance envelope.
- Cross-platform validation evidence.

**Exit Criteria**
- Installers run successfully on all supported platforms.
- Critical flows pass smoke, integration, and E2E validation.
- Bundle size and startup-time targets are understood and documented.

## Dependency Graph

```text
Phase 0 -> Phase 1 -> Phase 2 -> Phase 3
                           |          |
                           v          v
                        Phase 4    Phase 5
                           \          /
                            v        v
                             Phase 6
                                |
                                v
                             Phase 7
```

### Dependency Notes
- **Phase 0** must complete first because the build, shell, and CI foundation are prerequisites for all engineering work.
- **Phase 1** unlocks realistic testing and implementation for every downstream feature because optimization, back-test, import, and SLM explanations all depend on normalized data access and persistence.
- **Phase 2** is the first product-critical milestone and feeds both sensitivity analysis and rebalance capabilities.
- **Phase 3** depends on the optimization and data layers but can progress somewhat independently from the sensitivity UI once result models are established.
- **Phase 4** depends primarily on optimizer completeness and secondarily on charting patterns established in Phase 3.
- **Phase 5** depends on optimization plus stable persistence/import models, but can share validation and controller patterns from earlier phases.
- **Phase 6** should start only after recommendation and analytics outputs are stable enough to provide consistent explanation inputs.
- **Phase 7** consolidates everything and should not be treated as a catch-all for unfinished core logic.

## Risk Register

| Risk | Impact | Mitigation |
|------|--------|-----------|
| Qt Charts performance with large datasets | Medium | Downsample display series to ~500 points, keep full-resolution data for calculations/export, and avoid excessive QML re-binding during chart updates. |
| ONNX Runtime C++ build complexity | High | Prefer pre-built binaries via vcpkg/conan or pinned release artifacts; isolate discovery in `FindOnnxRuntime`; gate SLM support behind an option during early development. |
| Cross-platform Qt deployment | Medium | Automate packaging in CI, keep platform-specific assets under `packaging/`, and validate signing/bundle rules early instead of waiting for release week. |
| Alpha Vantage rate limits | Medium | Add aggressive caching, provider fallback routing, request coalescing, and visible retry/backoff logic so the app remains useful when throttled. |
| C++ build times | Medium | Use precompiled headers, ccache/sccache where available, modular targets, and avoid needless recompilation by keeping headers lean. |
| Provider schema drift or inconsistent financial fields | High | Centralize parsing adapters, add fixture-based contract tests, and capture normalized domain fields instead of leaking raw provider payloads. |
| Optimization edge cases produce infeasible portfolios | High | Implement rich diagnostics, infeasibility reporting, fallback heuristics, and strong test coverage around constraint boundaries. |
| Historical data gaps distort back-test metrics | High | Validate data completeness per symbol/window, annotate partial results, and enforce provider fallback plus cache invalidation rules. |
| File corruption from interrupted writes | Medium | Use temp-file-to-rename atomic writes, schema versioning, backup rotation for critical user files, and startup repair checks. |
| Local model size and inference latency hurt UX | Medium | Make SLM optional, support lightweight default models, stream or chunk explanation generation where possible, and surface progress/cancel states. |

### Ongoing Risk Management
- Review top risks at the end of each phase and update mitigations before starting the next phase.
- Treat provider reliability, optimizer correctness, and cross-platform packaging as release blockers.
- Keep sample datasets and deterministic fixtures under source control to catch regressions early.
- Track unresolved platform-specific issues separately from feature work so they are not lost inside general polish.

