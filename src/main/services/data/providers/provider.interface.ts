import type { Asset } from '@shared/types/asset.types.js';

export interface HistoricalPrice {
  date: string;
  open: number;
  high: number;
  low: number;
  close: number;
  adjustedClose: number;
  volume: number;
  dividendAmount: number;
}

export interface DividendEvent {
  date: string;
  amount: number;
  ticker: string;
}

export interface MarketDataProvider {
  readonly name: string;
  readonly requiresApiKey: boolean;

  /** Fetch current asset fundamentals (price, yield, beta) */
  getAssetInfo(ticker: string): Promise<Asset | null>;

  /** Fetch historical daily adjusted prices */
  getHistoricalPrices(ticker: string, startDate: string, endDate: string): Promise<HistoricalPrice[]>;

  /** Fetch dividend history */
  getDividendHistory(ticker: string, startDate: string, endDate: string): Promise<DividendEvent[]>;

  /** Fetch the current risk-free rate (only for FRED provider) */
  getRiskFreeRate?(): Promise<number>;

  /** Health check  verify API key works */
  healthCheck(): Promise<boolean>;
}