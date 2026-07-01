# Feature Specification: Dividend Tracker & Portfolio Management V2

**Feature Branch**: `feat/v2-dividend-tracker`  
**Created**: 2026-07-01  
**Status**: Draft  
**Input**: User description: "V2 expands the V1 yield-targeted portfolio optimizer into a full dividend-centric portfolio tracking and management system for global, multi-currency investors."

---

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Multi-Portfolio Ledger Management (Priority: P1)

As an income-focused investor, I want to create and manage multiple portfolios with transaction histories so I can track taxable, retirement, and watch-only accounts separately while still understanding my consolidated dividend income.

**Why this priority**: V2 only becomes a tracking product once users can persist real portfolios and update them over time. This is the foundation for dividend reporting, forecasting, analytics, tax handling, and alerts.

**Independent Test**: Create two portfolios, add buy/sell/DRIP transactions in different currencies, and verify that holdings, cost basis, and consolidated totals update correctly after restarting the app offline.

**Acceptance Scenarios**:

1. **Given** the user has no saved portfolios, **When** they create portfolios named "IRA" and "Taxable" with different base currencies, **Then** both portfolios are saved locally and appear in a consolidated portfolio selector.
2. **Given** a portfolio contains an existing holding, **When** the user records a buy transaction with trade date, quantity, price, fees, and currency, **Then** the holding quantity, average cost basis, and cash impact are recalculated and versioned in the local ledger.
3. **Given** a holding has accumulated shares, **When** the user records a sell transaction or a DRIP transaction, **Then** realized gain/loss, remaining lot balances, and reinvested share counts are updated without altering historical records.
4. **Given** the user opens the consolidated dashboard, **When** multiple portfolios hold the same ticker, **Then** the app shows both per-portfolio positions and a normalized consolidated position without double counting dividend events.

---

### User Story 2 - Dividend Income Tracking (Priority: P1)

As a dividend-focused investor, I want to see historical, current-year, and forward dividend income at holding, portfolio, and consolidated levels so I can understand whether my income stream is growing and predictable.

**Why this priority**: Dividend visibility is the core V2 value proposition. Without reliable income tracking, the product remains a generic portfolio viewer rather than a dividend management system.

**Independent Test**: Seed one portfolio with holdings that pay monthly and quarterly dividends, import historical payouts, and verify that the app correctly displays past income, year-to-date totals, and 12-month forward estimates.

**Acceptance Scenarios**:

1. **Given** a portfolio contains dividend-paying holdings, **When** the app syncs dividend histories, **Then** it displays gross income totals by month, quarter, year, holding, and portfolio.
2. **Given** a holding has irregular dividend dates, **When** new payout history is ingested, **Then** the system detects or updates the payout frequency and labels the estimate confidence appropriately.
3. **Given** the user enables DRIP tracking for a holding, **When** a dividend payout is marked as reinvested, **Then** the income ledger records both the cash-equivalent dividend and the resulting reinvestment shares.
4. **Given** the user switches from a single portfolio to consolidated view, **When** current-year income is shown, **Then** the app aggregates income across portfolios in the selected reporting currency while preserving source-currency drill-down.

---

### User Story 3 - Import, Export, Watchlists, and Sync Foundations (Priority: P1)

As a multi-portfolio manager, I want to import holdings from files, export my records, maintain watchlists, and refresh positions from repeatable sync sources so I can reduce manual entry and keep my local data current.

**Why this priority**: Real-world adoption depends on fast onboarding and manageable upkeep. Import/export and watchlists support both experienced users with existing broker records and new users building a tracked universe.

**Independent Test**: Import a CSV with holdings and transactions, export the resulting portfolio, add tickers to a watchlist, and rerun the sync source to confirm idempotent updates and duplicate prevention.

**Acceptance Scenarios**:

1. **Given** the user provides a supported CSV file with portfolio, holding, and transaction columns, **When** import completes, **Then** valid rows are persisted, invalid rows are reported with line-level errors, and no partial row is silently accepted.
2. **Given** a portfolio has already been imported once, **When** the same source file is synced again, **Then** duplicate transactions are detected by deterministic keys and skipped with a sync summary.
3. **Given** the user exports a portfolio, **When** they choose CSV export, **Then** the app outputs holdings, transactions, dividends, and summary metrics in a documented schema using local file storage only.
4. **Given** the user adds a ticker to a watchlist without owning it, **When** dividend and safety data refreshes run, **Then** the watchlist item appears in research and alerts without affecting portfolio income totals.

---

### User Story 4 - Dividend Safety Scoring (Priority: P2)

As a risk-aware investor, I want each holding to have a dividend safety score with clear risk categories and change alerts so I can identify potential cuts before they materially impact my income plan.

**Why this priority**: Once income is visible, the next user need is trusting that income. Safety scoring builds on tracked holdings and provider data but does not block the core P1 tracking loop.

**Independent Test**: Refresh fundamentals for a portfolio containing stable and deteriorating dividend stocks, verify the generated scores and categories, and confirm that a worsening score triggers an alert.

**Acceptance Scenarios**:

1. **Given** a holding has sufficient fundamentals and dividend history, **When** safety scoring runs, **Then** the app calculates a numeric score, maps it to safe/moderate/risky, and shows contributing factors.
2. **Given** a holding's debt, payout ratio, or dividend trend worsens on refresh, **When** the new score crosses a configured threshold, **Then** the user receives a local alert describing the reason for deterioration.
3. **Given** a holding lacks one or more required inputs, **When** scoring runs, **Then** the app marks the score as partial confidence and clearly identifies the missing data fields.
4. **Given** the user opens the holding detail view, **When** they inspect the score history, **Then** they can see past score snapshots and the deltas between refreshes.

---

### User Story 5 - Performance Analytics (Priority: P2)

As a data-driven investor, I want capital, dividend, currency, yield-on-cost, and total-return analytics so I can separate price gains from income growth and understand what is truly driving performance.

**Why this priority**: Analytics make tracked data actionable, but they depend on the ledger and dividend engine already being reliable. They meaningfully extend V1's back-testing into live portfolio evaluation.

**Independent Test**: Record transactions and dividend payouts for a multi-currency portfolio, then verify that analytics views correctly separate capital return, dividend return, FX impact, and total return across time ranges.

**Acceptance Scenarios**:

1. **Given** a portfolio has transactions, prices, and dividends, **When** the user opens analytics, **Then** the app displays capital gain/loss, dividend income, total return, and yield on cost for selectable periods.
2. **Given** a holding is denominated in a foreign currency, **When** the user views performance in their reporting currency, **Then** the app isolates exchange-rate impact from local-market price movement.
3. **Given** the user changes the date range, **When** analytics recompute, **Then** charts and summary cards stay synchronized and use the same valuation cut-off.
4. **Given** the user compares live performance with historical expectations, **When** they open a holding or portfolio detail, **Then** the app can reference reused V1 back-test outputs alongside realized results.

---

### User Story 6 - Diversification Dashboard (Priority: P2)

As a portfolio manager, I want to understand concentration risk across sectors, countries, currencies, and income sources so I can diversify my dividend stream and avoid hidden overexposure.

**Why this priority**: Diversification visibility becomes important once users trust the ledger and analytics. It is highly valuable but can be layered on after basic tracking and income reporting are stable.

**Independent Test**: Load a concentrated portfolio, refresh enrichment data, and verify that the dashboard identifies sector, issuer, geography, currency, and income concentration exposures.

**Acceptance Scenarios**:

1. **Given** the portfolio includes holdings across multiple sectors and countries, **When** the diversification dashboard loads, **Then** it displays allocations by market value and by forward dividend income.
2. **Given** one holding contributes an outsized share of income, **When** concentration analysis runs, **Then** the dashboard highlights the holding and shows its percentage of forward income and market value.
3. **Given** the same issuer is held across multiple portfolios, **When** the user switches to consolidated view, **Then** issuer exposure is aggregated correctly across accounts and currencies.
4. **Given** the user filters by portfolio or watchlist, **When** dashboard charts refresh, **Then** all slices, legends, and summary totals remain internally consistent.

---

### User Story 7 - Calendar & Event Awareness (Priority: P3)

As an investor planning monthly cash flow, I want a dividend and earnings calendar with notifications so I can anticipate declarations, ex-dividend dates, payout timing, and periods with income gaps.

**Why this priority**: Calendar awareness improves planning and engagement, but it depends on reliable holdings, dividend histories, and event ingestion already existing.

**Independent Test**: Populate a portfolio with several dividend-paying securities, sync event data, and verify the calendar, timeline, and local notifications for upcoming events.

**Acceptance Scenarios**:

1. **Given** the portfolio has upcoming dividend events, **When** the user opens the monthly calendar, **Then** declaration, ex-dividend, record, payable, and earnings events appear with portfolio context.
2. **Given** an event date changes after a data refresh, **When** the event pipeline updates, **Then** the app records the revision, updates the calendar, and flags the changed event in the timeline.
3. **Given** desktop notifications are enabled, **When** a user-defined reminder window is reached, **Then** a local notification is shown without sending portfolio data off-device.
4. **Given** the user views an empty month, **When** the calendar renders, **Then** it highlights expected low-income periods and offers a quick link to forward income projections.

---

### User Story 8 - Future Income Prediction & Scenario Modeling (Priority: P3)

As a planner, I want forward 12-month income projections with adjustable growth, DRIP, and tax assumptions so I can estimate future cash flow under best-case, base-case, and stress-case scenarios.

**Why this priority**: Forecasting depends on historical dividend data, payout frequencies, taxes, and holdings accuracy. It adds major planning value but should follow the core tracking and analytics layers.

**Independent Test**: Open a portfolio with dividend history, define scenario assumptions, and verify that monthly and annual projections change predictably when growth, DRIP, or tax inputs are modified.

**Acceptance Scenarios**:

1. **Given** a portfolio has forward dividend estimates, **When** the user opens the projection tool, **Then** the app displays a 12-month monthly income schedule and annual summary for the base scenario.
2. **Given** the user adjusts dividend growth rate, DRIP participation, or dividend cut stress assumptions, **When** the scenario recalculates, **Then** the projection updates immediately and preserves the prior scenario for comparison.
3. **Given** one or more holdings have uncertain payout frequencies, **When** the scenario runs, **Then** the projection labels those lines with lower confidence and explains the assumption used.
4. **Given** the user saves a scenario, **When** they return later offline, **Then** the saved assumptions and outputs remain available from local storage.

---

### User Story 9 - Tax-Aware Income Views (Priority: P3)

As an investor managing taxable income, I want withholding tax and net-income views per holding and portfolio so I can plan spendable income rather than only gross dividend receipts.

**Why this priority**: Tax handling is crucial for many dividend investors, especially across global markets, but it depends on the transaction ledger, dividend engine, and currency normalization already being in place.

**Independent Test**: Configure portfolio-level and holding-level withholding rules, process dividend events, and verify that gross, withheld, reclaimed, and net income values are calculated correctly.

**Acceptance Scenarios**:

1. **Given** a portfolio has a default withholding rate, **When** a new dividend is recorded for an eligible holding, **Then** gross, withholding, and net amounts are stored separately in the dividend ledger.
2. **Given** a holding has a custom withholding override, **When** dividend income is displayed, **Then** the holding uses its override instead of the portfolio default.
3. **Given** the user toggles between gross and net views, **When** charts and summary cards refresh, **Then** all analytics, forecasts, and exports consistently use the selected basis.
4. **Given** the portfolio spans multiple tax jurisdictions, **When** the user reviews a dividend event, **Then** the tax breakdown is shown in source currency and reporting currency with the applied FX rate.

---

### User Story 10 - News, Insights, and Local SLM Summaries (Priority: P4)

As a busy investor, I want a portfolio-filtered news feed, dividend change alerts, and plain-language weekly summaries generated locally so I can stay informed without reading every article manually.

**Why this priority**: This improves convenience and engagement, but the product still delivers its core value without it. It should reuse V1's local SLM infrastructure and only summarize already-fetched data.

**Independent Test**: Refresh news for tracked holdings, trigger a weekly summary, and verify that the feed is filtered, relevant dividend alerts are surfaced, and the SLM summary runs locally or falls back gracefully.

**Acceptance Scenarios**:

1. **Given** the user tracks a set of holdings, **When** the news feed refreshes, **Then** the app lists recent articles and company events filtered to those holdings and watchlist symbols.
2. **Given** a provider reports a dividend increase, cut, or suspension, **When** the event is ingested, **Then** the app raises a high-visibility insight card and links it to affected holdings and projections.
3. **Given** local SLM support is available, **When** the user requests a weekly summary, **Then** the app generates an on-device summary of dividend changes, notable risks, and upcoming cash-flow events.
4. **Given** no local model is installed, **When** the user requests a summary, **Then** the app offers a template-based digest and a local-model setup prompt without using any cloud LLM by default.

---

### User Story 11 - Research Lists and Basic Screening (Priority: P4)

As a learning or power user, I want top dividend stock lists, discovery lists, and simple screens so I can research candidates before adding them to a watchlist or portfolio.

**Why this priority**: Research tools expand discovery but are not required to manage existing portfolios. They should remain intentionally lightweight and reuse the existing provider and scoring foundations.

**Independent Test**: Open research tools, apply a simple dividend-growth screen, and verify that the returned results can be sorted, reviewed, and added to a watchlist.

**Acceptance Scenarios**:

1. **Given** research data is available, **When** the user opens top lists, **Then** the app shows curated lists such as high yield, dividend growth, and dividend aristocrat-style candidates with transparent definitions.
2. **Given** the user applies filters for yield, payout ratio, market cap, geography, and safety category, **When** the screen runs, **Then** results update and can be sorted without modifying the user's portfolios.
3. **Given** the user is reviewing a candidate from a research list, **When** they click "Add to Watchlist", **Then** the symbol is saved locally and becomes eligible for alerts and summaries.
4. **Given** a symbol lacks enough data for a filter, **When** the screen returns results, **Then** the app excludes or labels the symbol consistently instead of silently treating missing values as passing.

---

### Edge Cases

- What happens when the app is offline during launch? → Previously saved portfolios, dividend histories, scenarios, and cached analytics remain available read-only with staleness badges; refresh and news actions queue or fail gracefully.
- What happens when a provider returns partial fundamentals but complete dividend history? → Dividend tracking proceeds, safety scoring degrades to partial confidence, and missing fields are surfaced explicitly.
- What happens when two portfolios hold the same ticker in different currencies or on different exchanges? → Holdings remain distinct at the ledger level and are only consolidated after exchange-specific identity and FX normalization.
- How are special dividends handled? → They are stored as separate dividend event types and excluded from recurring forward-income estimates unless the user opts in.
- What happens when a company changes payout frequency from quarterly to monthly or skips a payment? → Frequency detection is recomputed, forecasts are revised, and confidence is lowered until the new pattern stabilizes.
- What happens when a DRIP reinvestment creates fractional shares below broker rounding precision? → Fractional precision is preserved per portfolio configuration and export formats declare their rounding rules.
- What happens when import data includes duplicate transactions with different memo text? → Deduplication uses stable business keys first and then surfaces ambiguous duplicates for manual review.
- What happens when a security changes ticker, merges, spins off, or delists? → Security aliases and corporate-action mappings preserve historical continuity while flagging positions that require user review.
- What happens when a holding pays in a currency different from the portfolio base currency? → Source-currency amounts are preserved, converted using stored FX snapshots, and displayed with both values in drill-down views.
- What happens when withholding tax rules change mid-year? → Tax rules are versioned by effective date and projections disclose which rule set was applied.
- What happens when event dates differ across providers? → Provider precedence and freshness rules select the displayed event while preserving the competing raw observations for auditability.
- What happens when news results contain duplicates from multiple providers? → Items are canonicalized by URL/content hash and shown once with source attribution.
- What happens when a local SLM model is unavailable or corrupted? → The app performs integrity checks, offers repair or re-download, and falls back to deterministic summaries.
- What happens when the user imports a malformed CSV with unsupported encodings or locale-specific decimal separators? → Import fails safely with actionable validation errors and locale-aware parsing guidance.
- What happens when a forecast would imply negative income because of extreme tax or cut assumptions? → The tool clamps invalid outputs, explains why, and requires the user to adjust assumptions.

---

## Requirements *(mandatory)*

### V1 Reuse & V2 Additions

**V1 components reused in V2:**
- Reuse the shared market-data provider abstraction and approved provider stack (Alpha Vantage primary, Financial Modeling Prep fallback, Polygon.io historical backfill, FRED macro series) as the only external market-data access path.
- Reuse the V1 back-test engine patterns, time-series utilities, and charting conventions where live performance, income growth, and scenario comparison need historical replay or benchmark context.
- Reuse the V1 file-based persistence approach: `electron-store` for preferences, `lowdb` for indexed collections, and atomic `fs.promises` JSON blobs for larger time-series payloads and imports.
- Reuse the V1 local SLM inference architecture, prompt orchestration, and fallback strategy for weekly summaries and insight generation.
- Reuse the V1 Electron split-process architecture, typed IPC boundaries, and shared runtime validation approach.

**New V2 components introduced by this specification:**
- A transaction-ledger domain for holdings, lots, dividends, taxes, watchlists, and sync jobs.
- Dividend-specific event ingestion, payout-frequency detection, and forward-income forecasting services.
- Dividend safety scoring, diversification analysis, calendar notifications, tax-aware projections, and research/news modules.
- Multi-portfolio consolidation and multi-currency normalization workflows built on top of V1 data and provider layers.

### Functional Requirements

#### Portfolio Tracking, Ledger, and Watchlists

- **FR-100**: System MUST allow users to create, rename, archive, and delete multiple portfolios stored locally.
- **FR-101**: System MUST support portfolio types including taxable, retirement, cash, watch-only, and custom labels.
- **FR-102**: System MUST record buy, sell, transfer-in, transfer-out, dividend, DRIP, fee, and cash-adjustment transactions with immutable history.
- **FR-103**: System MUST maintain holding quantities, lot-level cost basis, and realized gain/loss based on recorded transactions.
- **FR-104**: System MUST support fractional shares and configurable decimal precision per portfolio.
- **FR-105**: System MUST support watchlists independent of owned portfolios and allow symbols to exist in multiple watchlists.
- **FR-106**: System MUST provide consolidated views that aggregate the same economic exposure across portfolios without mutating source portfolio records.
- **FR-107**: System MUST preserve original trade currency, trade date, fees, and notes for every transaction.
- **FR-108**: System MUST support global equities and ETFs, including exchange metadata, local currency, and country/region attributes.
- **FR-109**: System MUST allow manual entry and editing of holdings and transactions when provider data is incomplete or unavailable.
- **FR-110**: System MUST maintain an auditable sync log for imports and refresh jobs, including created, updated, skipped, and failed records.

#### Import, Export, and Sync

- **FR-111**: System MUST import holdings and transactions from CSV files with configurable column mapping and preview-before-commit validation.
- **FR-112**: System MUST export holdings, transactions, dividend ledgers, and summary analytics to CSV.
- **FR-113**: System MUST detect duplicate imported transactions using deterministic identity rules and surface ambiguous duplicates for user review.
- **FR-114**: System MUST support repeatable local sync definitions so the user can rerun imports from known file schemas without remapping every time.
- **FR-115**: System MUST keep broker synchronization within local file import or explicitly approved read-only adapters; no background upload of portfolio data is allowed without user consent.

#### Dividend Tracking

- **FR-116**: System MUST store per-holding dividend event history including declaration date, ex-dividend date, record date, payable date, amount per share, currency, and event type.
- **FR-117**: System MUST calculate historical dividend income at holding, portfolio, and consolidated levels for selectable periods.
- **FR-118**: System MUST calculate current-year income totals and compare them with prior-year periods.
- **FR-119**: System MUST compute a forward 12-month gross dividend estimate per holding and per portfolio.
- **FR-120**: System MUST support DRIP by recording the gross dividend, withholding tax, reinvested cash amount, and resulting share increment.
- **FR-121**: System MUST detect payout frequency from historical events and classify estimates by confidence level when patterns are irregular.
- **FR-122**: System MUST distinguish regular, special, catch-up, corrected, and cancelled dividend events.
- **FR-123**: System MUST preserve dividend amounts in source currency and reporting currency using stored FX conversion snapshots.
- **FR-124**: System MUST display per-holding dividend history, including amount changes, gaps, and streak indicators.
- **FR-125**: System MUST allow manual correction or manual entry of dividend events while preserving the imported raw record for auditability.

#### Dividend Safety Score

- **FR-126**: System MUST calculate a dividend safety score for each eligible holding using configurable weighted inputs such as payout ratios, debt metrics, coverage, revenue/earnings trends, and dividend history.
- **FR-127**: System MUST map safety scores into at least three categories: safe, moderate, and risky.
- **FR-128**: System MUST expose score explanations showing the contributing factors, missing inputs, and confidence level.
- **FR-129**: System MUST persist score history so users can review deterioration or improvement over time.
- **FR-130**: System MUST generate local alerts when a holding's safety score crosses a user-defined deterioration threshold.

#### Calendar & Events

- **FR-131**: System MUST maintain a normalized event model for declarations, ex-dividend dates, record dates, payable dates, earnings dates, and user-created reminders.
- **FR-132**: System MUST render a monthly calendar and chronological timeline filtered by portfolio, watchlist, or consolidated scope.
- **FR-133**: System MUST support local desktop notifications for upcoming events with user-configurable reminder windows.
- **FR-134**: System MUST highlight event changes between refreshes and retain prior values for auditability.
- **FR-135**: System MUST allow users to dismiss or snooze notifications without deleting the underlying event.

#### Performance Analytics

- **FR-136**: System MUST calculate unrealized gain/loss, realized gain/loss, dividend income, total return, and yield on cost at holding, portfolio, and consolidated levels.
- **FR-137**: System MUST separate capital performance from dividend performance in both numeric summaries and charts.
- **FR-138**: System MUST calculate currency impact separately from local-market performance for non-base-currency holdings.
- **FR-139**: System MUST provide time-series analytics for portfolio value, income growth, rolling yield, and cumulative dividends.
- **FR-140**: System MUST support user-selectable reporting ranges including month-to-date, quarter-to-date, year-to-date, trailing 12 months, and custom ranges.
- **FR-141**: System MUST allow comparison between realized portfolio performance and reused V1 back-test or benchmark outputs when available.

#### Diversification Dashboard

- **FR-142**: System MUST display allocation by sector, industry, country/region, currency, asset type, and issuer.
- **FR-143**: System MUST display income concentration by holding, issuer, sector, and geography based on forward dividend income.
- **FR-144**: System MUST identify configurable concentration thresholds and flag breaches in the dashboard.
- **FR-145**: System MUST display dividend-growth distribution bands and safety-score distribution across holdings.
- **FR-146**: System MUST keep diversification totals internally consistent when filtered by portfolio, watchlist, or consolidated scope.

#### Future Income Prediction and Scenario Modeling

- **FR-147**: System MUST generate a forward 12-month monthly income schedule for each portfolio and the consolidated household view.
- **FR-148**: System MUST support user-defined scenario inputs including dividend growth rate, dividend cut stress, DRIP participation, contribution schedule, and tax basis.
- **FR-149**: System MUST support multiple saved scenarios per portfolio and allow side-by-side comparison.
- **FR-150**: System MUST indicate confidence or uncertainty for projections affected by sparse history, irregular payouts, or missing FX rates.
- **FR-151**: System MUST allow users to save scenario outputs locally for offline review.

#### Tax Handling

- **FR-152**: System MUST support portfolio-level default withholding tax settings and holding-level overrides.
- **FR-153**: System MUST store gross dividend, withheld amount, reclaimed amount, and net dividend as separate ledger values.
- **FR-154**: System MUST provide gross-versus-net toggles that consistently affect dashboards, analytics, forecasts, and exports.
- **FR-155**: System MUST version tax settings by effective date so historical dividend events are not retroactively rewritten.
- **FR-156**: System MUST display tax calculations in both source currency and selected reporting currency.

#### News, Insights, Research, and Local SLM Summaries

- **FR-157**: System MUST provide a portfolio- and watchlist-filtered news feed using only approved or separately approved public data sources exposed behind a provider interface.
- **FR-158**: System MUST detect and surface dividend increases, cuts, suspensions, resumptions, and notable earnings-related income risks as insight events.
- **FR-159**: System MUST deduplicate news items and insight events across providers while preserving source attribution.
- **FR-160**: System MUST provide curated research lists such as high-yield, dividend-growth, and user-saved screens with transparent ranking definitions.
- **FR-161**: System MUST provide basic screening filters for yield, dividend growth, payout ratio, sector, geography, market cap, currency, and safety category.
- **FR-162**: System MUST allow research candidates to be added to watchlists without creating owned positions.
- **FR-163**: System MUST generate weekly or on-demand summaries locally using the reused V1 SLM runtime, with a deterministic fallback when no valid local model is available.
- **FR-164**: System MUST never call a cloud LLM for summaries or insights unless the user explicitly opts in through a separate consent flow.

#### Data Providers, Currency, Persistence, and Architecture

- **FR-165**: System MUST access market and macro data only through the approved provider stack and shared provider interface reused from V1.
- **FR-166**: System MUST support provider fallback, rate limiting, caching, and schema validation for all external responses.
- **FR-167**: System MUST normalize and store FX rates needed for valuation and dividend conversion through the provider layer, with source attribution and timestamps.
- **FR-168**: System MUST persist preferences with `electron-store`, indexed collections with `lowdb`, and larger blobs with atomic JSON file writes; no database engine dependency is allowed.
- **FR-169**: System MUST separate main-process business logic, renderer UI, and shared types/schemas, with renderer-main communication occurring only through typed IPC.
- **FR-170**: System MUST operate offline for previously synced data and clearly mark stale or unavailable refresh-dependent content.

#### Security, Privacy, UX, and Testing

- **FR-171**: System MUST store API credentials and any sync secrets in OS-native secure storage rather than plain JSON files.
- **FR-172**: System MUST enforce HTTPS-only network access, CSP protections, and input validation for imported files and provider payloads.
- **FR-173**: System MUST ensure that no portfolio, transaction, or dividend data leaves the device without explicit user consent.
- **FR-174**: System MUST maintain a clean, ad-free, responsive desktop-first UI that remains usable at common laptop and large-monitor widths.
- **FR-175**: System MUST achieve fast initial dashboard rendering by loading cached summaries before non-critical enrichments.
- **FR-176**: System MUST follow test-first development: business logic requires failing unit tests first, provider and IPC boundaries require integration tests, and critical flows require Playwright end-to-end coverage.

### Key Entities

- **Portfolio**: A named locally stored account container with base currency, type, visibility state, reporting preferences, and references to holdings, transactions, scenarios, and alerts.
- **Holding**: The current position for a specific security within one portfolio, derived from transactions and enriched with market, dividend, currency, and classification metadata.
- **HoldingLot**: A lot-level record representing shares acquired together for cost-basis tracking, DRIP handling, and realized gain/loss calculations.
- **Transaction**: An immutable ledger event such as buy, sell, fee, cash adjustment, dividend cash receipt, or dividend reinvestment.
- **DividendEvent**: A normalized dividend record containing declaration/ex-dividend/record/payable dates, per-share amount, currency, event type, and source metadata.
- **DividendForecast**: A forward-looking estimate for expected dividend payments, including recurrence assumptions, confidence, source events, and scenario-adjusted outputs.
- **SafetyScoreSnapshot**: A time-stamped dividend safety assessment for a holding, including numeric score, category, factor breakdown, and missing-data notes.
- **PortfolioAnalyticsSnapshot**: A stored summary of market value, return decomposition, yield on cost, FX contribution, and income metrics for a specific cut-off date.
- **CalendarEvent**: A user-visible event for dividend, earnings, tax, or reminder workflows, with notification settings and revision history.
- **TaxProfile**: Portfolio- or holding-level withholding settings, effective dates, reclaim assumptions, and reporting preferences for gross or net views.
- **WatchlistItem**: A tracked symbol not necessarily owned, enriched with yield, safety, news, and screening metadata.
- **NewsInsight**: A deduplicated article or system-generated insight linked to holdings, watchlists, dividend events, or safety changes.
- **ScreenDefinition**: A user-saved research filter set defining the criteria for discovery lists and candidate ranking.
- **SyncJob**: A persisted import or refresh execution record with source details, timestamps, row outcomes, and retry-safe deduplication metadata.

---

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-100**: A user can create or import a portfolio with up to 500 transactions and view updated holdings in under 5 seconds on target hardware.
- **SC-101**: Historical and current-year dividend income totals render in under 2 seconds for a 100-holding consolidated view using warm local data.
- **SC-102**: Forward 12-month income projections recalculate in under 3 seconds after a scenario change for a 100-holding consolidated view.
- **SC-103**: Dividend safety scores refresh for 100 holdings in under 15 seconds excluding first-time provider cold-cache latency.
- **SC-104**: Diversification and performance dashboards load interactively in under 3 seconds from cached local summaries.
- **SC-105**: Calendar views display a full year of portfolio events in under 2 seconds from local data.
- **SC-106**: Gross-versus-net toggles update all visible analytics and projections without requiring an app restart and within 1 second for typical portfolios.
- **SC-107**: Local weekly summaries begin rendering within 1 second and complete within 5 seconds on supported target hardware when a local model is installed.
- **SC-108**: Offline relaunch preserves access to previously synced portfolios, dividends, scenarios, and alerts with no data loss after app restart.
- **SC-109**: Provider payload validation and import validation prevent silent corruption, with 100% of malformed rows or responses either rejected or explicitly quarantined.

---

## Assumptions

- V2 builds on top of the existing V1 optimizer, back-test engine, provider abstraction, SLM runtime, and file-based persistence rather than replacing them.
- The initial V2 release prioritizes tracking and management of existing portfolios; V1 portfolio generation and back-testing remain available as adjacent workflows.
- Multi-currency reporting uses provider-sourced FX rates stored locally; original source-currency values are never overwritten.
- Global market support focuses on publicly priced equities and ETFs first; more complex instruments such as options, bonds, and mutual-fund-specific accounting are out of scope for this spec.
- Broker synchronization in V2 is initially satisfied by repeatable local file imports and adapter-ready sync definitions; direct authenticated broker integrations require separate approval if they extend beyond the approved public market-data architecture.
- News and research modules remain lightweight and informational; they do not execute trades or provide fiduciary advice.
- All AI-generated summaries are local-first, optional, and explanatory only; they do not replace source data, calculations, or user judgment.
- Implementation should follow progressive delivery: P1 stories establish the dependable local ledger and dividend engine, then later priorities layer analysis, planning, and convenience features on top.
