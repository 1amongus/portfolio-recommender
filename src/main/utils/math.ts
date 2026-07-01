/**
 * Calculate annualized return from cumulative return and number of years
 */
export function annualizedReturn(cumulativeReturn: number, years: number): number {
  if (years <= 0) return 0;
  return (Math.pow(1 + cumulativeReturn, 1 / years) - 1);
}

/**
 * Calculate annualized volatility from array of daily returns
 */
export function annualizedVolatility(dailyReturns: number[]): number {
  if (dailyReturns.length < 2) return 0;
  const mean = dailyReturns.reduce((s, r) => s + r, 0) / dailyReturns.length;
  const variance = dailyReturns.reduce((s, r) => s + Math.pow(r - mean, 2), 0) / (dailyReturns.length - 1);
  return Math.sqrt(variance) * Math.sqrt(252); // 252 trading days
}

/**
 * Calculate maximum drawdown from equity curve
 */
export function maxDrawdown(equityCurve: number[]): number {
  let peak = equityCurve[0] || 0;
  let maxDd = 0;
  for (const value of equityCurve) {
    if (value > peak) peak = value;
    const dd = (peak - value) / peak;
    if (dd > maxDd) maxDd = dd;
  }
  return maxDd;
}

/**
 * Calculate Sharpe ratio
 */
export function sharpeRatio(annReturn: number, annVolatility: number, riskFreeRate: number): number {
  if (annVolatility === 0) return 0;
  return (annReturn - riskFreeRate) / annVolatility;
}

/**
 * Calculate Sortino ratio (uses downside deviation)
 */
export function sortinoRatio(annReturn: number, dailyReturns: number[], riskFreeRate: number): number {
  const dailyRfr = riskFreeRate / 252;
  const downsideReturns = dailyReturns.filter((r) => r < dailyRfr);
  if (downsideReturns.length === 0) return 0;
  const downsideVariance = downsideReturns.reduce((s, r) => s + Math.pow(r - dailyRfr, 2), 0) / downsideReturns.length;
  const downsideDeviation = Math.sqrt(downsideVariance) * Math.sqrt(252);
  if (downsideDeviation === 0) return 0;
  return (annReturn - riskFreeRate) / downsideDeviation;
}

/**
 * Calculate Beta relative to benchmark
 */
export function calculateBeta(assetReturns: number[], benchmarkReturns: number[]): number {
  if (assetReturns.length !== benchmarkReturns.length || assetReturns.length < 2) return 1;
  const n = assetReturns.length;
  const meanAsset = assetReturns.reduce((s, r) => s + r, 0) / n;
  const meanBench = benchmarkReturns.reduce((s, r) => s + r, 0) / n;

  let covariance = 0;
  let benchVariance = 0;
  for (let i = 0; i < n; i++) {
    covariance += (assetReturns[i] - meanAsset) * (benchmarkReturns[i] - meanBench);
    benchVariance += Math.pow(benchmarkReturns[i] - meanBench, 2);
  }

  if (benchVariance === 0) return 1;
  return covariance / benchVariance;
}