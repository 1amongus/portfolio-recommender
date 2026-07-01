import { contextBridge, ipcRenderer } from 'electron';
import { IPC_CHANNELS } from '../shared/types/ipc.types.js';

const api = {
  // Portfolio
  generatePortfolio: (request: unknown) =>
    ipcRenderer.invoke(IPC_CHANNELS.PORTFOLIO_GENERATE, request),
  listPortfolios: () =>
    ipcRenderer.invoke(IPC_CHANNELS.PORTFOLIO_LIST),
  savePortfolio: (portfolio: unknown) =>
    ipcRenderer.invoke(IPC_CHANNELS.PORTFOLIO_SAVE, portfolio),
  deletePortfolio: (id: string) =>
    ipcRenderer.invoke(IPC_CHANNELS.PORTFOLIO_DELETE, { id }),

  // Backtest
  runBacktest: (request: unknown) =>
    ipcRenderer.invoke(IPC_CHANNELS.BACKTEST_RUN, request),
  onBacktestProgress: (callback: (progress: unknown) => void) => {
    const listener = (_event: unknown, progress: unknown) => callback(progress);
    ipcRenderer.on(IPC_CHANNELS.BACKTEST_PROGRESS, listener);
    return () => ipcRenderer.removeListener(IPC_CHANNELS.BACKTEST_PROGRESS, listener);
  },

  // Sensitivity
  generateSensitivity: (request: unknown) =>
    ipcRenderer.invoke(IPC_CHANNELS.SENSITIVITY_GENERATE, request),
  onSensitivityProgress: (callback: (progress: unknown) => void) => {
    const listener = (_event: unknown, progress: unknown) => callback(progress);
    ipcRenderer.on(IPC_CHANNELS.SENSITIVITY_PROGRESS, listener);
    return () => ipcRenderer.removeListener(IPC_CHANNELS.SENSITIVITY_PROGRESS, listener);
  },

  // Settings
  getSettings: () => ipcRenderer.invoke(IPC_CHANNELS.SETTINGS_GET),
  setSettings: (settings: unknown) => ipcRenderer.invoke(IPC_CHANNELS.SETTINGS_SET, settings),
  getApiKey: (provider: string) => ipcRenderer.invoke(IPC_CHANNELS.SETTINGS_GET_API_KEY, provider),
  setApiKey: (provider: string, key: string) =>
    ipcRenderer.invoke(IPC_CHANNELS.SETTINGS_SET_API_KEY, { provider, key }),
};

contextBridge.exposeInMainWorld('electronAPI', api);

export type ElectronAPI = typeof api;