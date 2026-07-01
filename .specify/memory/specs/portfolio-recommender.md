# Feature Specification: Portfolio Recommender

**Feature Branch**: `feat/portfolio-recommender-core`  
**Created**: 2026-07-01  
**Status**: Draft  
**Input**: User description: "Electron desktop app for yield-targeted portfolio optimization using Beta minimization, back-testing, portfolio import/rebalance, and Yield & Beta sensitivity curves."

---

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Yield-Targeted Portfolio Generation (Priority: P1)

As an investor, I want to specify a desired dividend yield percentage and have the system automatically construct an optimized portfolio of up to five stocks or ETFs that collectively meet my yield target while minimizing overall Beta (market sensitivity).

**Why this priority**: This is the core value proposition of the application. Without yield-targeted portfolio generation, the product has no reason to exist. Every other feature builds on top of this engine.

**Independent Test**: Can be fully tested by entering a yield target (e.g., 4.5%) and verifying that the system returns a portfolio of ≤5 assets that meets the yield requirement with minimized aggregate Beta. Delivers immediate, actionable investment insight.

**Acceptance Scenarios**:

1. **Given** the user is on the Portfolio Generation screen, **When** they enter a target yield of 4.0% and click "Generate", **Then** the system displays a portfolio of 1–5 stocks/ETFs with a weighted yield ≥ 4.0% and the lowest achievable portfolio Beta.
2. **Given** the optimization engine has access to market data, **When** a yield target of 2.5% is submitted, **Then** the result includes ticker symbols, allocation weights (summing to 100%), individual yields, individual Betas, and the aggregate portfolio Beta.
3. **Given** a yield target that is unreasonably high (e.g., 25%), **When** the user submits it, **Then** the system displays a warning explaining that the target exceeds available assets and suggests the maximum achievable yield.
4. **Given** API data is stale or unavailable for certain tickers, **When** optimization runs, **Then** those tickers are excluded and the user is notified which assets were dropped.

---

### User Story 2 - Historical Back-Testing (Priority: P1)

As an investor, I want to back-test any recommended portfolio against historical market data to understand how it would have performed in terms of total return, drawdown, yield stability, and risk-adjusted metrics across different market conditions.

**Why this priority**: Back-testing is essential for user trust. Recommendations without historical validation are speculative. This feature is co-equal to generation because users will not act on recommendations they cannot verify.

**Independent Test**: Can be tested by generating a portfolio (or using a hardcoded test portfolio) and running a back-test over a defined date range. Verify that the output includes cumulative return, annualized return, max drawdown, Sharpe ratio, Sortino ratio, and yield consistency metrics.

**Acceptance Scenarios**:

1. **Given** a generated portfolio of 3 ETFs, **When** the user selects a back-test period of 5 years and clicks "Run Back-Test", **Then** the system displays an equity curve, annual returns, max drawdown, Sharpe ratio, Sortino ratio, and yield-on-cost over time.
2. **Given** a back-test is running, **When** data is being fetched and computed, **Then** a progress indicator shows estimated time remaining and the UI remains responsive.
3. **Given** a back-test period that extends before an asset's IPO date, **When** the back-test runs, **Then** that asset is excluded from periods where it did not exist and the user is informed.
4. **Given** a completed back-test, **When** the user views results, **Then** they can toggle between chart views (equity curve, drawdown chart, rolling yield chart) and a tabular summary.

---

### User Story 3 - Portfolio Import & Rebalance (Priority: P2)

As an investor with an existing portfolio, I want to import my current holdings (tickers and weights or dollar amounts) so the system can generate a rebalanced version that aligns with my target yield while minimizing Beta, and then compare the rebalanced portfolio against my original via back-testing.

**Why this priority**: Import/rebalance converts the app from a "what if" tool into a personal portfolio advisor. It requires Story 1 and Story 2 to be functional, but is the key differentiator that drives retention.

**Independent Test**: Can be tested by importing a CSV or manually entering 3–10 holdings, specifying a yield target, and verifying that the system produces a rebalanced portfolio (≤5 assets) along with a side-by-side back-test comparison.

**Acceptance Scenarios**:

1. **Given** the user is on the Import screen, **When** they upload a CSV file with columns `ticker, shares` (or `ticker, weight`), **Then** the system parses, validates tickers, fetches current prices/yields/Betas, and displays the imported portfolio summary.
2. **Given** an imported portfolio and a target yield of 3.5%, **When** the user clicks "Rebalance", **Then** the system generates an optimized portfolio (≤5 assets) meeting the yield target with minimized Beta and displays it alongside the original.
3. **Given** both original and rebalanced portfolios exist, **When** the user clicks "Compare Back-Tests", **Then** the system runs back-tests on both and displays overlaid equity curves, comparative metrics table, and a summary of improvements (yield delta, Beta reduction, drawdown improvement).
4. **Given** the CSV contains invalid tickers, **When** parsing completes, **Then** invalid entries are highlighted with suggestions (fuzzy match) and the user can correct or remove them before proceeding.
5. **Given** manual entry mode is selected, **When** the user types a ticker, **Then** autocomplete suggests matching symbols from the supported universe.

---

### User Story 4 - Yield & Beta Sensitivity Curve (Priority: P2)

As an investor, I want to view a Yield & Beta sensitivity curve that shows how my portfolio's Beta changes as I adjust the target yield, so I can understand the trade-off between income and volatility and choose the right yield target for my risk tolerance.

**Why this priority**: This visualization transforms raw numbers into intuitive decision-making. It's the analytical differentiator that makes the app feel professional and trustworthy. Depends on the optimization engine (Story 1) being functional.

**Independent Test**: Can be tested by selecting a yield range (e.g., 1%–8%) and verifying that the system generates a curve plotting achievable yield on the X-axis against minimum portfolio Beta on the Y-axis, with an interactive tooltip showing the portfolio composition at each point.

**Acceptance Scenarios**:

1. **Given** the user navigates to the Sensitivity Analysis screen, **When** they specify a yield range (min: 1%, max: 8%, step: 0.25%) and click "Generate Curve", **Then** the system computes an optimized portfolio at each yield step and renders a smooth curve of Yield (X) vs. Beta (Y).
2. **Given** the sensitivity curve is displayed, **When** the user hovers over any point on the curve, **Then** a tooltip shows the yield, Beta, and the specific portfolio composition (tickers + weights) for that point.
3. **Given** the sensitivity curve is displayed, **When** the user clicks a point on the curve, **Then** that portfolio is loaded into the Generation view for further analysis or back-testing.
4. **Given** the user has imported a portfolio, **When** they view the sensitivity curve, **Then** the current portfolio's yield/Beta is plotted as a distinct marker on the curve for comparison.
5. **Given** computation takes more than 2 seconds, **When** the curve is being generated, **Then** a progress bar shows completion percentage (one increment per yield step computed).

---

### User Story 5 - SLM-Powered Insights & Explanations (Priority: P3)

As an investor, I want the application to generate plain-language explanations of recommendations, risk assessments, and market context summaries using a local Small Language Model, so I can understand the reasoning behind portfolio selections without needing financial expertise.

**Why this priority**: SLM integration adds a layer of accessibility and polish but is not structurally required for the core quantitative features. It can be added incrementally once the numerical engine is solid.

**Independent Test**: Can be tested by generating a portfolio and verifying that a natural-language summary is produced explaining why each asset was selected, what risk factors are present, and how the portfolio compares to a benchmark.

**Acceptance Scenarios**:

1. **Given** a portfolio has been generated, **When** the user clicks "Explain Recommendation", **Then** the local SLM produces a 150–300 word summary explaining asset selection rationale, Beta contribution of each holding, and overall risk profile.
2. **Given** a back-test has completed, **When** the user clicks "Summarize Performance", **Then** the SLM generates a narrative describing key periods (drawdowns, recoveries), comparisons to benchmarks, and yield stability commentary.
3. **Given** the SLM model is not yet downloaded, **When** the user triggers an explanation, **Then** the system prompts to download the model (showing size and estimated time) or offers a fallback template-based explanation.
4. **Given** the SLM is generating text, **When** inference is in progress, **Then** tokens stream into the UI progressively (typewriter effect) with a cancel button.

---

### User Story 6 - Persistent Portfolio Management (Priority: P3)

As a returning user, I want my generated portfolios, imported portfolios, back-test results, and sensitivity analyses to be saved locally so I can review and compare them across sessions without re-running computations.

**Why this priority**: Persistence is a quality-of-life feature. The app functions without it (re-run each time), but long-term users need session continuity.

**Independent Test**: Can be tested by generating a portfolio, closing the app, reopening it, and verifying the portfolio and its back-test results are still accessible.

**Acceptance Scenarios**:

1. **Given** the user generates a portfolio, **When** they close and reopen the app, **Then** the portfolio appears in a "Saved Portfolios" list with its metadata (date created, yield target, Beta).
2. **Given** multiple saved portfolios exist, **When** the user opens the Saved Portfolios view, **Then** they can sort by date, yield, or Beta, and delete or rename entries.
3. **Given** a saved portfolio's underlying data is outdated (>30 days), **When** the user opens it, **Then** a badge indicates "stale data" with an option to refresh with current market data.

---

### Edge Cases

- What happens when the market data API is completely unreachable at launch? → App enters offline mode showing cached data with staleness warnings; optimization disabled until connectivity restored.
- What happens when all available assets have Beta > 1.0? → System returns the lowest-Beta combination available with a warning that market sensitivity cannot be reduced below the displayed level.
- What happens when the user's yield target can only be met with a single asset? → System returns a 1-asset portfolio with a diversification warning.
- How does the system handle stock splits, delistings, or ticker changes in historical data? → Back-test engine uses adjusted prices; delisted assets are excluded from periods after delisting with a notation.
- What if the user's imported portfolio contains assets not covered by the data APIs? → Those assets are flagged as "unsupported" with the option to manually input yield/Beta or exclude them.
- What happens when the SLM produces a hallucinated or nonsensical explanation? → Explanations are rendered with a disclaimer: "AI-generated summary — verify with source data." A "Report Issue" button logs the prompt/response for debugging.
- What if the local SLM model file is corrupted? → Integrity check on startup; if corrupted, prompt re-download; template fallback available.

---

## Requirements *(mandatory)*

### Functional Requirements

**Portfolio Generation Engine:**
- **FR-001**: System MUST accept a target yield percentage (0.1%–30%) as input.
- **FR-002**: System MUST construct a portfolio of no more than 5 assets (stocks or ETFs) that meets or exceeds the target yield.
- **FR-003**: System MUST minimize the weighted portfolio Beta subject to the yield constraint.
- **FR-004**: System MUST use a constrained optimization algorithm (e.g., quadratic programming, SLSQP, or equivalent) for asset selection and weight allocation.
- **FR-005**: System MUST source dividend yield and Beta data from the following public API stack: Alpha Vantage (primary, fundamentals + daily prices), Financial Modeling Prep (fallback, same scope), Polygon.io (bulk historical price backfill), and FRED (risk-free rate for Sharpe/Sortino). All providers accessed via the shared `MarketDataProvider` interface.
- **FR-006**: System MUST display individual asset details (ticker, name, weight, yield, Beta) and aggregate portfolio metrics.
- **FR-007**: System MUST validate that weights sum to 100% (±0.01% tolerance for floating-point).
- **FR-008**: System MUST support a configurable asset universe (default: S&P 500 constituents + top 100 ETFs by AUM).

**Back-Testing Module:**
- **FR-009**: System MUST accept a date range for back-testing (minimum 1 year, maximum constrained by data availability).
- **FR-010**: System MUST compute: cumulative return, annualized return, annualized volatility, maximum drawdown, Sharpe ratio (risk-free rate configurable), Sortino ratio, and yield-on-cost over time.
- **FR-011**: System MUST use adjusted close prices (accounting for splits and dividends) for return calculations.
- **FR-012**: System MUST render an interactive equity curve chart with zoom, pan, and date-range selection.
- **FR-013**: System MUST support monthly rebalancing assumption in back-tests (configurable: monthly, quarterly, annually, never).

**Portfolio Import & Rebalance:**
- **FR-014**: System MUST accept portfolio input via CSV upload (columns: ticker, shares OR ticker, weight) or manual entry.
- **FR-015**: System MUST validate imported tickers against the supported asset universe and flag unrecognized entries.
- **FR-016**: System MUST compute current yield and Beta for the imported portfolio.
- **FR-017**: System MUST generate a rebalanced portfolio (≤5 assets) targeting the user's specified yield with minimized Beta.
- **FR-018**: System MUST display side-by-side comparison of original vs. rebalanced portfolios with delta metrics.
- **FR-019**: System MUST run comparative back-tests on both portfolios with overlaid visualizations.

**Sensitivity Analysis:**
- **FR-020**: System MUST generate a Yield vs. Beta curve over a user-specified yield range with configurable step size.
- **FR-021**: System MUST render an interactive chart where each point reveals the underlying portfolio composition.
- **FR-022**: System MUST mark the user's current portfolio (if imported) on the sensitivity curve for reference.
- **FR-023**: System MUST allow the user to click any curve point to load that portfolio for further analysis.

**SLM Integration:**
- **FR-024**: System MUST run SLM inference locally using ONNX Runtime or llama.cpp node bindings.
- **FR-025**: System MUST provide plain-language explanations for generated portfolios, back-test results, and sensitivity trade-offs.
- **FR-026**: System MUST support streaming token output to the UI during inference.
- **FR-027**: System MUST allow users to select and download SLM models from a curated list (with size/capability metadata).
- **FR-028**: System MUST gracefully degrade to template-based explanations when no SLM model is available.

**Data & Persistence:**
- **FR-029**: System MUST cache API responses locally as individual JSON files (one per response, keyed by content hash) with configurable TTL (default: 24 hours for price data, 7 days for fundamental data). Writes use atomic temp-file-then-rename pattern.
- **FR-030**: System MUST persist portfolios, back-test results, and user preferences using a file-based JSON architecture: electron-store for preferences, lowdb for indexed collections (portfolios, assets, sensitivity curves), and atomic fs.promises writes for large blobs (backtests in userData/backtests/, cache in userData/cache/). No database engine dependency.
- **FR-031**: System MUST allow export of portfolio data and back-test results to CSV and PDF.
- **FR-032**: System MUST indicate data staleness (>30 days) with visual badges and refresh prompts.

**Application Shell:**
- **FR-033**: System MUST run as a cross-platform Electron desktop application (Windows, macOS, Linux).
- **FR-034**: System MUST support auto-update via electron-updater.
- **FR-035**: System MUST store API keys in OS-native secure storage (Windows Credential Manager, macOS Keychain, Linux Secret Service).
- **FR-036**: System MUST provide a first-run setup wizard for API key configuration.
- **FR-037**: System MUST enforce Content Security Policy in the renderer process.
- **FR-038**: System MUST reject any non-HTTPS network requests at the application layer; HTTP URLs in provider configuration MUST cause a validation error.
- **FR-039**: System MUST NOT include any telemetry, analytics, or usage tracking. Future telemetry features require explicit opt-in consent UI and a constitution amendment.
- **FR-040**: System MUST validate all API response payloads against expected schemas (Zod runtime validation) before processing; malformed responses MUST be rejected with a logged warning and user notification.

### Key Entities

- **Portfolio**: A named collection of assets with allocation weights, a target yield, computed aggregate Beta, and creation metadata. Can be system-generated or user-imported. Stored in `portfolios.json` (lowdb). References assets by ID and backtests by ID.
- **Asset**: A stock or ETF identified by ticker symbol, with attributes: name, sector, current price, dividend yield, trailing yield, Beta, and market cap. Stored in `assets.json` (lowdb).
- **BackTestResult**: The output of a historical simulation, linked to a Portfolio via ID reference, containing time-series data (equity curve, rolling metrics) and summary statistics. Stored as individual JSON files in `userData/backtests/{id}.json` (atomic fs.promises).
- **SensitivityCurve**: A set of (yield, Beta, portfolio) tuples representing the efficient frontier of yield/Beta trade-offs for the current asset universe. Stored in `sensitivity.json` (lowdb).
- **UserPreferences**: Application settings including API keys (reference to secure storage), default yield range, preferred back-test period, rebalancing frequency, and SLM model selection. Stored via `electron-store` in `preferences.json`.
- **SLMModel**: Metadata for a locally-stored language model: name, file path, size, quantization level, capability description, and integrity hash.

---

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Users can generate an optimized portfolio within 10 seconds of submitting a yield target (excluding first-time API cold cache).
- **SC-002**: Back-test results for a 5-year period render within 15 seconds on target hardware (8-core CPU, 16GB RAM).
- **SC-003**: Sensitivity curve (30 yield steps) computes and renders within 45 seconds.
- **SC-004**: Portfolio import (up to 50 holdings) parses, validates, and displays within 5 seconds.
- **SC-005**: SLM-generated explanations complete within 5 seconds (streaming begins within 1 second) on target hardware.
- **SC-006**: Application cold-starts to interactive UI in under 3 seconds.
- **SC-007**: Total application memory footprint remains under 1GB (excluding SLM model weights loaded during inference).
- **SC-008**: Back-tested portfolios match manual calculations within ±0.5% for cumulative return (validation against known benchmarks).
- **SC-009**: Application functions in offline mode (cached data) for all features except fresh portfolio generation requiring live API data.
- **SC-010**: Zero unhandled exceptions reaching the user — all error states produce actionable messages.

---

## Assumptions

- Users have internet connectivity for initial data fetch; offline mode relies on cached data from prior sessions.
- Target users are self-directed retail investors with basic financial literacy (understand yield, diversification) but not necessarily quantitative finance expertise.
- Public APIs (Alpha Vantage for fundamentals/daily prices, Financial Modeling Prep as fallback, Polygon.io for bulk historical backfill, FRED for risk-free rate) provide sufficient data (≥10 years daily adjusted close + dividend history) for the supported asset universe. Alpha Vantage's 25 req/day free-tier limit is mitigated by aggressive caching and Polygon.io's grouped daily bars endpoint for bulk backfill.
- The asset universe (S&P 500 + top ETFs) provides adequate coverage for yield targets between 1%–10%; extreme targets outside this range may produce warnings.
- Mobile and web versions are out of scope for v1; this is desktop-only.
- SLM models in the 1B–4B parameter range (quantized to 4-bit) provide acceptable explanation quality on consumer hardware.
- Users will supply their own API keys during initial setup (free-tier keys are sufficient for typical usage patterns with caching).
- Beta is calculated relative to S&P 500 (SPY) using 2-year trailing weekly returns as the default lookback configuration.
- Dividend yield is calculated as trailing 12-month dividends divided by current price.
- The optimization engine does not account for transaction costs, taxes, or minimum lot sizes in v1 — these are noted as future enhancements.
