export interface Asset {
  ticker: string;
  name: string;
  sector: string;
  currentPrice: number;
  dividendYield: number;
  trailingYield: number;
  beta: number;
  marketCap: number;
  exchange: string;
  lastUpdated: string;
}

export interface AssetUniverse {
  assets: Asset[];
  lastRefreshed: string;
  source: string;
}
