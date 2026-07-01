import type { PortfolioHolding } from './portfolio.types.js';

export interface SensitivityRequest {
  yieldMin: number;
  yieldMax: number;
  yieldStep: number;
}

export interface SensitivityPoint {
  targetYield: number;
  achievedYield: number;
  beta: number;
  holdings: PortfolioHolding[];
  feasible: boolean;
}

export interface SensitivityCurve {
  id: string;
  points: SensitivityPoint[];
  computedAt: string;
  computeTimeMs: number;
}

export interface SensitivityProgress {
  currentStep: number;
  totalSteps: number;
  percentComplete: number;
}
