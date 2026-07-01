import { useState, useCallback } from 'react';
import type { ElectronAPI } from '../../preload/index.js';

declare global {
  interface Window {
    electronAPI: ElectronAPI;
  }
}

export function useIpc<TRequest, TResponse>(
  method: (request: TRequest) => Promise<TResponse>,
) {
  const [data, setData] = useState<TResponse | null>(null);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const invoke = useCallback(
    async (request: TRequest) => {
      setLoading(true);
      setError(null);
      try {
        const result = await method(request);
        setData(result);
        return result;
      } catch (err) {
        const message = err instanceof Error ? err.message : 'Unknown error';
        setError(message);
        throw err;
      } finally {
        setLoading(false);
      }
    },
    [method],
  );

  return { data, loading, error, invoke };
}