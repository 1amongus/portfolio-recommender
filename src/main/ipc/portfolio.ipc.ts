import { ipcMain } from 'electron';
import { IPC_CHANNELS } from '@shared/types/ipc.types.js';
import { generatePortfolioSchema } from '@shared/validation/schemas.js';
import { generateOptimizedPortfolio } from '../services/optimization/optimizer.js';
import { getPortfolios, savePortfolio, deletePortfolio } from '../services/data/store.js';

export function registerIpcHandlers(): void {
  ipcMain.handle(IPC_CHANNELS.PORTFOLIO_GENERATE, async (_event, request) => {
    const parsed = generatePortfolioSchema.parse(request);
    return generateOptimizedPortfolio(parsed.targetYield, parsed.maxAssets, parsed.excludeTickers);
  });

  ipcMain.handle(IPC_CHANNELS.PORTFOLIO_LIST, async () => {
    return getPortfolios();
  });

  ipcMain.handle(IPC_CHANNELS.PORTFOLIO_SAVE, async (_event, portfolio) => {
    await savePortfolio(portfolio);
    return { success: true };
  });

  ipcMain.handle(IPC_CHANNELS.PORTFOLIO_DELETE, async (_event, { id }) => {
    const deleted = await deletePortfolio(id);
    return { success: deleted };
  });
}