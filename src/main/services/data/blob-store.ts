import { promises as fs } from 'fs';
import { join } from 'path';
import { app } from 'electron';
import { createHash } from 'crypto';
import type { BacktestResult } from '@shared/types/backtest.types.js';

function getUserDataPath(): string {
  return app.getPath('userData');
}

// Atomic write: write to .tmp then rename
async function atomicWrite(filePath: string, data: string): Promise<void> {
  const tmp = `${filePath}.tmp`;
  await fs.writeFile(tmp, data, 'utf8');
  await fs.rename(tmp, filePath);
}

// Backtest blob operations
export async function saveBacktest(result: BacktestResult): Promise<void> {
  const dir = join(getUserDataPath(), 'backtests');
  const filePath = join(dir, `${result.id}.json`);
  await atomicWrite(filePath, JSON.stringify(result));
}

export async function loadBacktest(id: string): Promise<BacktestResult | null> {
  const filePath = join(getUserDataPath(), 'backtests', `${id}.json`);
  try {
    const data = await fs.readFile(filePath, 'utf8');
    return JSON.parse(data) as BacktestResult;
  } catch {
    return null;
  }
}

export async function deleteBacktest(id: string): Promise<boolean> {
  const filePath = join(getUserDataPath(), 'backtests', `${id}.json`);
  try {
    await fs.unlink(filePath);
    return true;
  } catch {
    return false;
  }
}

export async function listBacktestIds(): Promise<string[]> {
  const dir = join(getUserDataPath(), 'backtests');
  try {
    const files = await fs.readdir(dir);
    return files.filter((f) => f.endsWith('.json')).map((f) => f.replace('.json', ''));
  } catch {
    return [];
  }
}

// Cache operations
function cacheKey(url: string, params?: Record<string, string>): string {
  const input = url + JSON.stringify(params ?? {});
  return createHash('sha256').update(input).digest('hex');
}

interface CacheEntry<T = unknown> {
  data: T;
  cachedAt: number;
  ttlMs: number;
  url: string;
}

export async function getCached<T>(url: string, params?: Record<string, string>): Promise<T | null> {
  const key = cacheKey(url, params);
  const filePath = join(getUserDataPath(), 'cache', `${key}.json`);
  try {
    const raw = await fs.readFile(filePath, 'utf8');
    const entry = JSON.parse(raw) as CacheEntry<T>;
    if (Date.now() - entry.cachedAt < entry.ttlMs) {
      return entry.data;
    }
    return null; // expired
  } catch {
    return null;
  }
}

export async function setCache<T>(
  url: string,
  data: T,
  ttlMs: number,
  params?: Record<string, string>,
): Promise<void> {
  const key = cacheKey(url, params);
  const filePath = join(getUserDataPath(), 'cache', `${key}.json`);
  const entry: CacheEntry<T> = { data, cachedAt: Date.now(), ttlMs, url };
  await atomicWrite(filePath, JSON.stringify(entry));
}