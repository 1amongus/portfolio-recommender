import { JSONFilePreset } from 'lowdb/node';
import { app } from 'electron';
import { join } from 'path';
import { mkdir } from 'fs/promises';
import type { Portfolio } from '@shared/types/portfolio.types.js';
import type { Asset } from '@shared/types/asset.types.js';
import type { SensitivityCurve } from '@shared/types/sensitivity.types.js';

export interface DbSchema {
  portfolios: Portfolio[];
  assets: Asset[];
  sensitivityCurves: SensitivityCurve[];
}

const DEFAULT_DATA: DbSchema = {
  portfolios: [],
  assets: [],
  sensitivityCurves: [],
};

let db: Awaited<ReturnType<typeof JSONFilePreset<DbSchema>>> | null = null;

export async function initializeStorage(): Promise<void> {
  const userDataPath = app.getPath('userData');
  
  // Ensure directories exist
  await mkdir(join(userDataPath, 'backtests'), { recursive: true });
  await mkdir(join(userDataPath, 'cache'), { recursive: true });

  const dbPath = join(userDataPath, 'portfolios.json');
  db = await JSONFilePreset<DbSchema>(dbPath, DEFAULT_DATA);
}

export function getDb(): NonNullable<typeof db> {
  if (!db) throw new Error('Storage not initialized. Call initializeStorage() first.');
  return db;
}

// Portfolio operations
export async function getPortfolios(): Promise<Portfolio[]> {
  return getDb().data.portfolios;
}

export async function savePortfolio(portfolio: Portfolio): Promise<void> {
  const existing = getDb().data.portfolios.findIndex((p) => p.id === portfolio.id);
  if (existing >= 0) {
    getDb().data.portfolios[existing] = portfolio;
  } else {
    getDb().data.portfolios.push(portfolio);
  }
  await getDb().write();
}

export async function deletePortfolio(id: string): Promise<boolean> {
  const idx = getDb().data.portfolios.findIndex((p) => p.id === id);
  if (idx < 0) return false;
  getDb().data.portfolios.splice(idx, 1);
  await getDb().write();
  return true;
}

// Asset operations
export async function getAssets(): Promise<Asset[]> {
  return getDb().data.assets;
}

export async function updateAssets(assets: Asset[]): Promise<void> {
  getDb().data.assets = assets;
  await getDb().write();
}