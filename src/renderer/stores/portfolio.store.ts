import { create } from 'zustand';
import type { Portfolio, GeneratePortfolioResult } from '@shared/types/portfolio.types.js';

interface PortfolioState {
  currentResult: GeneratePortfolioResult | null;
  savedPortfolios: Portfolio[];
  isGenerating: boolean;
  error: string | null;
  
  setResult: (result: GeneratePortfolioResult) => void;
  setSavedPortfolios: (portfolios: Portfolio[]) => void;
  setGenerating: (loading: boolean) => void;
  setError: (error: string | null) => void;
  clear: () => void;
}

export const usePortfolioStore = create<PortfolioState>((set) => ({
  currentResult: null,
  savedPortfolios: [],
  isGenerating: false,
  error: null,

  setResult: (result) => set({ currentResult: result, error: null }),
  setSavedPortfolios: (portfolios) => set({ savedPortfolios: portfolios }),
  setGenerating: (loading) => set({ isGenerating: loading }),
  setError: (error) => set({ error }),
  clear: () => set({ currentResult: null, error: null }),
}));