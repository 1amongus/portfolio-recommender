export interface AppSettings {
  theme: 'light' | 'dark' | 'system';
  defaultYieldMin: number;
  defaultYieldMax: number;
  defaultBacktestYears: number;
  rebalanceFrequency: 'monthly' | 'quarterly' | 'annually' | 'never';
  cacheTtlPriceHours: number;
  cacheTtlFundamentalDays: number;
  slmModelId: string | null;
  windowBounds: { x: number; y: number; width: number; height: number } | null;
}

export const DEFAULT_SETTINGS: AppSettings = {
  theme: 'dark',
  defaultYieldMin: 1.0,
  defaultYieldMax: 8.0,
  defaultBacktestYears: 5,
  rebalanceFrequency: 'quarterly',
  cacheTtlPriceHours: 24,
  cacheTtlFundamentalDays: 7,
  slmModelId: null,
  windowBounds: null,
};

export type ApiProvider = 'alpha-vantage' | 'fmp' | 'polygon' | 'fred';

export interface ApiKeyConfig {
  provider: ApiProvider;
  key: string;
  configured: boolean;
}
