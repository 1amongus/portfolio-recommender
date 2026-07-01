import type { Asset } from '@shared/types/asset.types.js';
import type { Portfolio, PortfolioHolding, GeneratePortfolioResult, PortfolioWarning } from '@shared/types/portfolio.types.js';
import { MAX_PORTFOLIO_ASSETS } from '@shared/constants.js';
import { getAssets } from '../data/store.js';
import { randomUUID } from 'crypto';

interface OptimizationCandidate {
  asset: Asset;
  yieldContribution: number;
}

/**
 * Greedy Beta-minimization optimizer.
 * 
 * Strategy:
 * 1. Filter universe to assets meeting minimum yield threshold
 * 2. Sort candidates by beta (ascending)  prefer low-beta assets
 * 3. Greedily select assets that collectively meet yield target
 * 4. Optimize weights to minimize portfolio beta while maintaining yield constraint
 */
export async function generateOptimizedPortfolio(
  targetYield: number,
  maxAssets: number = MAX_PORTFOLIO_ASSETS,
  excludeTickers: string[] = [],
): Promise<GeneratePortfolioResult> {
  const startTime = Date.now();
  const warnings: PortfolioWarning[] = [];
  const allAssets = await getAssets();

  // Filter universe
  const candidates = allAssets.filter(
    (a) =>
      a.dividendYield > 0 &&
      a.beta > 0 &&
      !excludeTickers.includes(a.ticker) &&
      isDataFresh(a.lastUpdated),
  );

  if (candidates.length === 0) {
    throw new Error('No eligible assets found in the universe. Refresh market data first.');
  }

  // Check max achievable yield
  const sortedByYield = [...candidates].sort((a, b) => b.dividendYield - a.dividendYield);
  const maxAchievableYield = sortedByYield.slice(0, maxAssets).reduce((sum, a) => sum + a.dividendYield, 0) / Math.min(maxAssets, sortedByYield.length);

  if (targetYield > maxAchievableYield) {
    warnings.push({
      type: 'yield-cap',
      message: `Target yield ${targetYield}% exceeds maximum achievable ${maxAchievableYield.toFixed(2)}%. Returning best available portfolio.`,
      details: { maxAchievableYield },
    });
  }

  // Select assets: sort by beta ascending, pick those that can contribute to yield
  const selected = selectAssets(candidates, targetYield, maxAssets);

  if (selected.length === 1) {
    warnings.push({
      type: 'single-asset',
      message: 'Only one asset meets the criteria. Portfolio is not diversified.',
    });
  }

  // Optimize weights
  const holdings = optimizeWeights(selected, targetYield);

  const aggregateYield = holdings.reduce((sum, h) => sum + h.weight * h.yield, 0) / 100;
  const aggregateBeta = holdings.reduce((sum, h) => sum + h.weight * h.beta, 0) / 100;

  const portfolio: Portfolio = {
    id: randomUUID(),
    name: `Yield ${targetYield}% Portfolio`,
    holdings,
    targetYield,
    aggregateYield: aggregateYield * 100,
    aggregateBeta: aggregateBeta * 100,
    createdAt: new Date().toISOString(),
    updatedAt: new Date().toISOString(),
    source: 'generated',
  };

  return {
    portfolio,
    warnings,
    computeTimeMs: Date.now() - startTime,
  };
}

function selectAssets(candidates: Asset[], targetYield: number, maxAssets: number): Asset[] {
  // Sort by beta ascending (prefer low-beta)
  const sortedByBeta = [...candidates].sort((a, b) => a.beta - b.beta);

  const selected: Asset[] = [];
  let weightedYield = 0;

  for (const asset of sortedByBeta) {
    if (selected.length >= maxAssets) break;

    selected.push(asset);
    // Equal-weight estimate
    weightedYield = selected.reduce((sum, a) => sum + a.dividendYield, 0) / selected.length;

    if (weightedYield >= targetYield && selected.length >= 2) {
      break;
    }
  }

  // If we haven't met yield target, swap in higher-yield assets
  if (weightedYield < targetYield) {
    const highYieldCandidates = candidates
      .filter((a) => !selected.includes(a))
      .sort((a, b) => b.dividendYield - a.dividendYield);

    for (const candidate of highYieldCandidates) {
      if (selected.length >= maxAssets) {
        // Replace highest-beta asset if candidate improves yield without too much beta
        const highestBetaIdx = selected.reduce(
          (maxIdx, a, idx) => (a.beta > selected[maxIdx].beta ? idx : maxIdx),
          0,
        );
        if (candidate.dividendYield > selected[highestBetaIdx].dividendYield) {
          selected[highestBetaIdx] = candidate;
        }
      } else {
        selected.push(candidate);
      }

      weightedYield = selected.reduce((sum, a) => sum + a.dividendYield, 0) / selected.length;
      if (weightedYield >= targetYield) break;
    }
  }

  return selected;
}

function optimizeWeights(assets: Asset[], targetYield: number): PortfolioHolding[] {
  if (assets.length === 0) return [];
  if (assets.length === 1) {
    return [
      {
        ticker: assets[0].ticker,
        name: assets[0].name,
        weight: 100,
        yield: assets[0].dividendYield,
        beta: assets[0].beta,
      },
    ];
  }

  // Iterative weight optimization: minimize beta while meeting yield target
  // Start with equal weights, then shift weight toward lower-beta assets
  const n = assets.length;
  let weights = new Array(n).fill(100 / n);

  // Simple gradient descent: shift weight from high-beta to low-beta assets
  // while maintaining yield >= target
  for (let iteration = 0; iteration < 100; iteration++) {
    const portfolioBeta = weights.reduce((sum, w, i) => sum + w * assets[i].beta, 0) / 100;
    const portfolioYield = weights.reduce((sum, w, i) => sum + w * assets[i].dividendYield, 0) / 100;

    // Find highest-beta and lowest-beta asset indices
    let maxBetaIdx = 0;
    let minBetaIdx = 0;
    for (let i = 1; i < n; i++) {
      if (assets[i].beta > assets[maxBetaIdx].beta) maxBetaIdx = i;
      if (assets[i].beta < assets[minBetaIdx].beta) minBetaIdx = i;
    }

    if (maxBetaIdx === minBetaIdx) break;

    // Try shifting 1% from high-beta to low-beta
    const step = 1;
    if (weights[maxBetaIdx] <= step) break;

    const newWeights = [...weights];
    newWeights[maxBetaIdx] -= step;
    newWeights[minBetaIdx] += step;

    const newYield = newWeights.reduce((sum, w, i) => sum + w * assets[i].dividendYield, 0) / 100;

    if (newYield >= targetYield) {
      weights = newWeights;
    } else {
      break; // Can't shift more without violating yield constraint
    }
  }

  // Normalize weights to sum to exactly 100
  const totalWeight = weights.reduce((sum, w) => sum + w, 0);
  weights = weights.map((w) => (w / totalWeight) * 100);

  return assets.map((asset, i) => ({
    ticker: asset.ticker,
    name: asset.name,
    weight: Math.round(weights[i] * 100) / 100,
    yield: asset.dividendYield,
    beta: asset.beta,
  }));
}

function isDataFresh(lastUpdated: string): boolean {
  const age = Date.now() - new Date(lastUpdated).getTime();
  return age < 30 * 24 * 60 * 60 * 1000; // 30 days
}