import type { Asset } from '@shared/types/asset.types.js';
import type { MarketDataProvider, HistoricalPrice, DividendEvent } from './provider.interface.js';
import { getCached, setCache } from '../blob-store.js';
import { CACHE_TTL_PRICE_MS, CACHE_TTL_FUNDAMENTAL_MS } from '@shared/constants.js';

const BASE_URL = 'https://www.alphavantage.co/query';

export class AlphaVantageProvider implements MarketDataProvider {
  readonly name = 'alpha-vantage';
  readonly requiresApiKey = true;

  constructor(private apiKey: string) {}

  async getAssetInfo(ticker: string): Promise<Asset | null> {
    const cacheUrl = `${BASE_URL}/OVERVIEW/${ticker}`;
    const cached = await getCached<Asset>(cacheUrl);
    if (cached) return cached;

    try {
      const url = `${BASE_URL}?function=OVERVIEW&symbol=${ticker}&apikey=${this.apiKey}`;
      const response = await fetch(url);
      if (!response.ok) return null;
      const data = await response.json();

      if (!data.Symbol) return null;

      const asset: Asset = {
        ticker: data.Symbol,
        name: data.Name || ticker,
        sector: data.Sector || 'Unknown',
        currentPrice: parseFloat(data.AnalystTargetPrice) || 0,
        dividendYield: parseFloat(data.DividendYield) * 100 || 0,
        trailingYield: parseFloat(data.TrailingAnnualDividendYield) * 100 || 0,
        beta: parseFloat(data.Beta) || 1.0,
        marketCap: parseFloat(data.MarketCapitalization) || 0,
        exchange: data.Exchange || 'Unknown',
        lastUpdated: new Date().toISOString(),
      };

      await setCache(cacheUrl, asset, CACHE_TTL_FUNDAMENTAL_MS);
      return asset;
    } catch (error) {
      console.error(`Alpha Vantage getAssetInfo error for ${ticker}:`, error);
      return null;
    }
  }

  async getHistoricalPrices(ticker: string, startDate: string, endDate: string): Promise<HistoricalPrice[]> {
    const cacheUrl = `${BASE_URL}/TIME_SERIES_DAILY_ADJUSTED/${ticker}/${startDate}/${endDate}`;
    const cached = await getCached<HistoricalPrice[]>(cacheUrl);
    if (cached) return cached;

    try {
      const url = `${BASE_URL}?function=TIME_SERIES_DAILY_ADJUSTED&symbol=${ticker}&outputsize=full&apikey=${this.apiKey}`;
      const response = await fetch(url);
      if (!response.ok) return [];
      const data = await response.json();

      const timeSeries = data['Time Series (Daily)'];
      if (!timeSeries) return [];

      const prices: HistoricalPrice[] = Object.entries(timeSeries)
        .filter(([date]) => date >= startDate && date <= endDate)
        .map(([date, values]: [string, any]) => ({
          date,
          open: parseFloat(values['1. open']),
          high: parseFloat(values['2. high']),
          low: parseFloat(values['3. low']),
          close: parseFloat(values['4. close']),
          adjustedClose: parseFloat(values['5. adjusted close']),
          volume: parseInt(values['6. volume']),
          dividendAmount: parseFloat(values['7. dividend amount']),
        }))
        .sort((a, b) => a.date.localeCompare(b.date));

      await setCache(cacheUrl, prices, CACHE_TTL_PRICE_MS);
      return prices;
    } catch (error) {
      console.error(`Alpha Vantage getHistoricalPrices error for ${ticker}:`, error);
      return [];
    }
  }

  async getDividendHistory(ticker: string, startDate: string, endDate: string): Promise<DividendEvent[]> {
    // Dividends are included in the daily adjusted response
    const prices = await this.getHistoricalPrices(ticker, startDate, endDate);
    return prices
      .filter((p) => p.dividendAmount > 0)
      .map((p) => ({
        date: p.date,
        amount: p.dividendAmount,
        ticker,
      }));
  }

  async healthCheck(): Promise<boolean> {
    try {
      const url = `${BASE_URL}?function=GLOBAL_QUOTE&symbol=SPY&apikey=${this.apiKey}`;
      const response = await fetch(url);
      const data = await response.json();
      return !!data['Global Quote']?.['01. symbol'];
    } catch {
      return false;
    }
  }
}