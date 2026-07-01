# Portfolio Recommender Constitution

## Core Principles

### I. Native Desktop-First (Qt)

This is a cross-platform desktop application built with Qt 6 LTS. All features must be designed for the desktop context: native OS integration, offline-capable where feasible, responsive windowed UI, and local data persistence. The application uses Qt/QML for a declarative interface and C++17 for native application logic, system integration, and background work. Native rendering and Qt's cross-platform abstraction are required; no browser engine is part of the runtime.

### II. Public API Consumer

The application exclusively consumes public financial market APIs. The approved provider stack is:
- **Primary**: Alpha Vantage (fundamentals: dividend yield, Beta, company overview, daily prices)
- **Fallback**: Financial Modeling Prep (same data scope, alternative source)
- **Supplement**: Polygon.io (bulk historical price backfill for back-testing via grouped endpoints)
- **Macro**: FRED (risk-free rate / 10Y Treasury for Sharpe & Sortino calculations)

No proprietary or paid-tier-only data sources without explicit amendment. All API integrations must be abstracted behind a provider interface to allow swapping data sources. Rate limiting, caching, and graceful degradation on API failure are mandatory.

### III. Local SLM Integration

Small Language Models (SLMs) run locally on-device for inference tasks such as summarization, sentiment analysis, and recommendation explanation. Models must be lightweight enough for consumer hardware (target: 4GB RAM overhead max). Use native C++ integrations through the ONNX Runtime C++ API or llama.cpp. No cloud-based LLM calls unless the user explicitly opts in. Model selection and updating must be user-configurable.

### IV. Test-First (NON-NEGOTIABLE)

TDD is mandatory for all business logic: tests written first, validated to fail, then implementation proceeds. Red-Green-Refactor cycle strictly enforced. Unit tests for all services and utilities; integration tests for API provider contracts, data persistence, and C++/QML boundaries; E2E tests (Squish or Appium) for critical user flows. Qt Test and CTest are the required foundation for automated validation.

### V. Security & Privacy by Default

No user financial data leaves the device without explicit consent. API keys stored in OS-native secure storage via Qt Keychain. All network requests use HTTPS and must be configured with appropriate QSslConfiguration defaults. Input validation on all external data. No browser CSP is required because the application does not embed a web renderer for core UI. No telemetry without opt-in. Dependency audit on every build.

### VI. Separation of Concerns

Clear architectural boundaries enforced:
- **Models (C++ backend)**: API orchestration, SLM inference, persistence, secure storage, domain logic
- **Views (QML)**: UI components, presentation, visualization, declarative bindings
- **Controllers (C++ exposed to QML)**: coordination logic surfaced via Q_PROPERTY, signals/slots, and Q_INVOKABLE entry points
- Signals/slots are the only communication mechanism between layers; no shared mutable state and no ad hoc cross-layer access

### VII. Simplicity & Progressive Complexity

Start simple, ship incrementally. Core recommendation engine must function with a single API source before multi-source aggregation is added. YAGNI applies — no speculative abstractions. Features gated behind progressive disclosure in the UI. Prefer the simplest Qt/C++ tool that satisfies the requirement before introducing additional framework layers or dependencies.

## Technology Stack

| Layer | Technology |
|-------|-----------|
| Shell | Qt 6 LTS |
| Frontend | QML (Qt Quick) |
| State | Qt Property System + Q_PROPERTY bindings |
| Styling | QML styling + Qt Quick Controls Material/Universal theme |
| Charts | Qt Charts module (hardware-accelerated via Scene Graph) |
| Local DB | QSettings (preferences) + QJsonDocument + QFile (atomic writes) |
| SLM Runtime | ONNX Runtime C++ API / llama.cpp native |
| Testing | Qt Test (unit/integration), Squish or Appium (E2E), CTest |
| Packaging | CPack / Qt Installer Framework |
| Linting | clang-tidy + clang-format |
| Build | CMake 3.21+ |

Dependencies must be justified. Prefer Qt-native and standard C++ solutions before introducing third-party libraries unless performance, maintainability, or platform support clearly require them.

## Development Workflow

1. **Branching**: Feature branches off `main`; short-lived (< 1 week). Naming: `feat/`, `fix/`, `chore/`.
2. **Commits**: Conventional Commits enforced. Atomic commits — one logical change per commit.
3. **Code Review**: All changes require review. Reviewer verifies test coverage, security implications, and constitution compliance.
4. **CI Pipeline**: clang-format → clang-tidy → Qt Test → Build → E2E tests. All gates must pass before merge.
5. **Releases**: Semantic versioning. Installer packaging via CPack or Qt Installer Framework. Changelog generated from conventional commits.

## Performance Standards

- Cold start to interactive UI: < 2 seconds
- API data refresh: non-blocking via QNetworkAccessManager async
- SLM inference latency: < 5 seconds for recommendation generation on target hardware (8-core CPU, 16GB RAM)
- Memory ceiling: < 500MB total application footprint (excluding model weights)
- Bundle size: < 50MB installed

## Governance

This constitution supersedes all other project practices and conventions. Any amendment requires:
1. Written proposal with rationale
2. Impact analysis on existing code
3. Migration plan for affected components
4. Approval documented in version history

All code contributions must verify compliance with these principles. Complexity must be justified against the Simplicity principle. When principles conflict, priority order is: Security > Correctness > Simplicity > Performance.

**Version**: 2.0.0 | **Ratified**: 2026-07-01 | **Last Amended**: 2026-07-01
**Amendment Note**: 2.0.0 — Migrated from Electron/TypeScript to Qt/QML/C++17 for native performance and reduced footprint.
