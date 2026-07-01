# Portfolio Recommender Constitution

## Core Principles

### I. Electron Desktop-First

This is a cross-platform desktop application built with Electron. All features must be designed for the desktop context: native OS integration, offline-capable where feasible, responsive windowed UI, and local data persistence. The renderer process uses a modern frontend framework (React + TypeScript). The main process handles system-level concerns, IPC, and background tasks.

### II. Public API Consumer

The application exclusively consumes public financial market APIs. The approved provider stack is:
- **Primary**: Alpha Vantage (fundamentals: dividend yield, Beta, company overview, daily prices)
- **Fallback**: Financial Modeling Prep (same data scope, alternative source)
- **Supplement**: Polygon.io (bulk historical price backfill for back-testing via grouped endpoints)
- **Macro**: FRED (risk-free rate / 10Y Treasury for Sharpe & Sortino calculations)

No proprietary or paid-tier-only data sources without explicit amendment. All API integrations must be abstracted behind a provider interface to allow swapping data sources. Rate limiting, caching, and graceful degradation on API failure are mandatory.

### III. Local SLM Integration

Small Language Models (SLMs) run locally on-device for inference tasks such as summarization, sentiment analysis, and recommendation explanation. Models must be lightweight enough for consumer hardware (target: 4GB RAM overhead max). Use ONNX Runtime or llama.cpp for local inference. No cloud-based LLM calls unless the user explicitly opts in. Model selection and updating must be user-configurable.

### IV. Test-First (NON-NEGOTIABLE)

TDD is mandatory for all business logic: tests written first, validated to fail, then implementation proceeds. Red-Green-Refactor cycle strictly enforced. Unit tests for all services and utilities; integration tests for API provider contracts and IPC boundaries; E2E tests (Playwright/Spectron) for critical user flows.

### V. Security & Privacy by Default

No user financial data leaves the device without explicit consent. API keys stored in OS-native secure storage (keychain/credential manager). All network requests use HTTPS. Input validation on all external data. CSP headers enforced in renderer. No telemetry without opt-in. Dependency audit on every build.

### VI. Separation of Concerns

Clear architectural boundaries enforced:
- **Main process**: API orchestration, SLM inference, secure storage, system tray, auto-update
- **Renderer process**: UI components, state management, visualization
- **Shared**: Type definitions, constants, validation schemas
- IPC is the only communication channel between main and renderer; no shared mutable state

### VII. Simplicity & Progressive Complexity

Start simple, ship incrementally. Core recommendation engine must function with a single API source before multi-source aggregation is added. YAGNI applies — no speculative abstractions. Features gated behind progressive disclosure in the UI.

## Technology Stack

| Layer | Technology |
|-------|-----------|
| Shell | Electron (latest stable) |
| Frontend | React 18+, TypeScript 5+, Vite |
| State | Zustand |
| Styling | Tailwind CSS |
| Charts | Recharts or D3.js |
| Local DB | File-based JSON (electron-store, lowdb, atomic fs.promises) |
| SLM Runtime | ONNX Runtime / llama.cpp (node bindings) |
| Testing | Vitest (unit), Playwright (E2E) |
| Packaging | electron-builder |
| Linting | ESLint + Prettier |

Dependencies must be justified. No native modules required for storage — all persistence is pure JS. Prefer pure JS/TS packages where performance allows.

## Development Workflow

1. **Branching**: Feature branches off `main`; short-lived (< 1 week). Naming: `feat/`, `fix/`, `chore/`.
2. **Commits**: Conventional Commits enforced. Atomic commits — one logical change per commit.
3. **Code Review**: All changes require review. Reviewer verifies test coverage, security implications, and constitution compliance.
4. **CI Pipeline**: Lint → Type-check → Unit tests → Integration tests → Build → E2E tests. All gates must pass before merge.
5. **Releases**: Semantic versioning. Auto-update via electron-updater. Changelog generated from conventional commits.

## Performance Standards

- Cold start to interactive UI: < 3 seconds
- API data refresh: non-blocking, with skeleton/loading states
- SLM inference latency: < 5 seconds for recommendation generation on target hardware (8-core CPU, 16GB RAM)
- Memory ceiling: < 1GB total application footprint (excluding model weights)
- Bundle size: renderer JS < 2MB gzipped

## Governance

This constitution supersedes all other project practices and conventions. Any amendment requires:
1. Written proposal with rationale
2. Impact analysis on existing code
3. Migration plan for affected components
4. Approval documented in version history

All code contributions must verify compliance with these principles. Complexity must be justified against the Simplicity principle. When principles conflict, priority order is: Security > Correctness > Simplicity > Performance.

**Version**: 1.0.0 | **Ratified**: 2026-07-01 | **Last Amended**: 2026-07-01
