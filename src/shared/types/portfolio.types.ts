import type { Asset } from './asset.types.js';

export interface PortfolioHolding {
  ticker: string;
  name: string;
  weight: number;
  yield: number;
  beta: number;
}

export interface Portfolio {
  id: string;
  name: string;
  holdings: PortfolioHolding[];
  targetYield: number;
  aggregateYield: number;
  aggregateBeta: number;
  createdAt: string;
  updatedAt: string;
  source: 'generated' | 'imported' | 'rebalanced';
}

export interface GeneratePortfolioRequest {
  targetYield: number;
  maxAssets?: number;
  excludeTickers?: string[];
}

export interface GeneratePortfolioResult {
  portfolio: Portfolio;
  warnings: PortfolioWarning[];
  computeTimeMs: number;
}

export interface PortfolioWarning {
  type: 'yield-cap' | 'single-asset' | 'high-beta' | 'stale-data' | 'dropped-assets';
  message: string;
  details?: Record<string, unknown>;
}
