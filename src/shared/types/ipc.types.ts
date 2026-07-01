import type { GeneratePortfolioRequest, GeneratePortfolioResult, Portfolio } from './portfolio.types.js';
import type { BacktestRequest, BacktestResult, BacktestProgress } from './backtest.types.js';
import type { SensitivityRequest, SensitivityCurve, SensitivityProgress } from './sensitivity.types.js';

export const IPC_CHANNELS = {
  // Portfolio
  PORTFOLIO_GENERATE: 'portfolio:generate',
  PORTFOLIO_LIST: 'portfolio:list',
  PORTFOLIO_SAVE: 'portfolio:save',
  PORTFOLIO_DELETE: 'portfolio:delete',

  // Backtest
  BACKTEST_RUN: 'backtest:run',
  BACKTEST_PROGRESS: 'backtest:progress',

  // Sensitivity
  SENSITIVITY_GENERATE: 'sensitivity:generate',
  SENSITIVITY_PROGRESS: 'sensitivity:progress',

  // Settings
  SETTINGS_GET: 'settings:get',
  SETTINGS_SET: 'settings:set',
  SETTINGS_GET_API_KEY: 'settings:getApiKey',
  SETTINGS_SET_API_KEY: 'settings:setApiKey',

  // Import
  IMPORT_CSV: 'import:csv',
  IMPORT_VALIDATE: 'import:validate',

  // SLM
  SLM_EXPLAIN: 'slm:explain',
  SLM_STREAM: 'slm:stream',
  SLM_CANCEL: 'slm:cancel',
} as const;

export type IpcChannel = typeof IPC_CHANNELS[keyof typeof IPC_CHANNELS];

export interface IpcHandlerMap {
  [IPC_CHANNELS.PORTFOLIO_GENERATE]: { request: GeneratePortfolioRequest; response: GeneratePortfolioResult };
  [IPC_CHANNELS.PORTFOLIO_LIST]: { request: void; response: Portfolio[] };
  [IPC_CHANNELS.PORTFOLIO_SAVE]: { request: Portfolio; response: { success: boolean } };
  [IPC_CHANNELS.PORTFOLIO_DELETE]: { request: { id: string }; response: { success: boolean } };
  [IPC_CHANNELS.BACKTEST_RUN]: { request: BacktestRequest; response: BacktestResult };
  [IPC_CHANNELS.SENSITIVITY_GENERATE]: { request: SensitivityRequest; response: SensitivityCurve };
}
