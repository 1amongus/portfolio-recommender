export const APP_NAME = 'Portfolio Recommender';
export const APP_VERSION = '1.0.0';

export const MAX_PORTFOLIO_ASSETS = 5;
export const MIN_YIELD_TARGET = 0.1;
export const MAX_YIELD_TARGET = 30.0;

export const CACHE_TTL_PRICE_MS = 24 * 60 * 60 * 1000; // 24 hours
export const CACHE_TTL_FUNDAMENTAL_MS = 7 * 24 * 60 * 60 * 1000; // 7 days
export const STALE_DATA_THRESHOLD_MS = 30 * 24 * 60 * 60 * 1000; // 30 days

export const BETA_LOOKBACK_WEEKS = 104; // 2 years
export const BETA_BENCHMARK_TICKER = 'SPY';

export const SUPPORTED_PROVIDERS = ['alpha-vantage', 'fmp', 'polygon', 'fred'] as const;
