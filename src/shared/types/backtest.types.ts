export interface BacktestRequest {
  portfolioId: string;
  startDate: string;
  endDate: string;
  rebalanceFrequency: 'monthly' | 'quarterly' | 'annually' | 'never';
  riskFreeRate?: number;
}

export interface BacktestMetrics {
  cumulativeReturn: number;
  annualizedReturn: number;
  annualizedVolatility: number;
  maxDrawdown: number;
  sharpeRatio: number;
  sortinoRatio: number;
  yieldOnCost: number;
}

export interface BacktestTimePoint {
  date: string;
  portfolioValue: number;
  cumulativeReturn: number;
  drawdown: number;
  rollingYield: number;
}

export interface BacktestResult {
  id: string;
  portfolioId: string;
  request: BacktestRequest;
  metrics: BacktestMetrics;
  timeSeries: BacktestTimePoint[];
  computedAt: string;
  computeTimeMs: number;
}

export interface BacktestProgress {
  phase: 'fetching' | 'computing' | 'complete';
  percentComplete: number;
  currentDate?: string;
}
