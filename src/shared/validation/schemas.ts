import { z } from 'zod';

export const generatePortfolioSchema = z.object({
  targetYield: z.number().min(0.1).max(30),
  maxAssets: z.number().int().min(1).max(5).optional().default(5),
  excludeTickers: z.array(z.string().toUpperCase()).optional().default([]),
});

export const backtestRequestSchema = z.object({
  portfolioId: z.string().min(1),
  startDate: z.string().regex(/^\d{4}-\d{2}-\d{2}$/),
  endDate: z.string().regex(/^\d{4}-\d{2}-\d{2}$/),
  rebalanceFrequency: z.enum(['monthly', 'quarterly', 'annually', 'never']),
  riskFreeRate: z.number().min(0).max(20).optional(),
});

export const sensitivityRequestSchema = z.object({
  yieldMin: z.number().min(0.1).max(30),
  yieldMax: z.number().min(0.1).max(30),
  yieldStep: z.number().min(0.1).max(5),
}).refine((data) => data.yieldMax > data.yieldMin, {
  message: 'yieldMax must be greater than yieldMin',
});

export const urlSchema = z.string().url().startsWith('https://', {
  message: 'Only HTTPS URLs are allowed per security policy',
});

export const tickerSchema = z.string().min(1).max(10).regex(/^[A-Z0-9.]+$/);
