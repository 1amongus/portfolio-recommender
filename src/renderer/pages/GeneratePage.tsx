import { useState } from 'react';
import { usePortfolioStore } from '../stores/portfolio.store.js';

export function GeneratePage() {
  const [targetYield, setTargetYield] = useState(4.0);
  const { currentResult, isGenerating, error, setResult, setGenerating, setError } =
    usePortfolioStore();

  const handleGenerate = async () => {
    setGenerating(true);
    setError(null);
    try {
      const result = await window.electronAPI.generatePortfolio({ targetYield });
      setResult(result);
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Generation failed');
    } finally {
      setGenerating(false);
    }
  };

  return (
    <div className="max-w-4xl">
      <h2 className="text-2xl font-bold mb-6">Generate Portfolio</h2>

      {/* Input Section */}
      <div className="bg-gray-800 rounded-lg p-6 mb-6">
        <label className="block text-sm font-medium text-gray-300 mb-2">
          Target Dividend Yield (%)
        </label>
        <div className="flex gap-4 items-end">
          <input
            type="number"
            min={0.1}
            max={30}
            step={0.1}
            value={targetYield}
            onChange={(e) => setTargetYield(parseFloat(e.target.value) || 0)}
            className="w-32 px-3 py-2 bg-gray-700 border border-gray-600 rounded-md text-white focus:ring-2 focus:ring-primary-500 focus:border-transparent"
          />
          <button
            onClick={handleGenerate}
            disabled={isGenerating}
            className="px-6 py-2 bg-primary-600 hover:bg-primary-700 disabled:bg-gray-600 text-white font-medium rounded-md transition-colors"
          >
            {isGenerating ? 'Generating...' : 'Generate'}
          </button>
        </div>
      </div>

      {/* Error */}
      {error && (
        <div className="bg-red-900/50 border border-red-700 text-red-200 px-4 py-3 rounded-lg mb-6">
          {error}
        </div>
      )}

      {/* Results */}
      {currentResult && (
        <div className="bg-gray-800 rounded-lg p-6">
          <div className="flex justify-between items-center mb-4">
            <h3 className="text-lg font-semibold">Optimized Portfolio</h3>
            <div className="flex gap-4 text-sm">
              <span className="text-green-400">
                Yield: {currentResult.portfolio.aggregateYield.toFixed(2)}%
              </span>
              <span className="text-blue-400">
                Beta: {currentResult.portfolio.aggregateBeta.toFixed(3)}
              </span>
              <span className="text-gray-400">
                {currentResult.computeTimeMs}ms
              </span>
            </div>
          </div>

          {/* Warnings */}
          {currentResult.warnings.length > 0 && (
            <div className="mb-4 space-y-2">
              {currentResult.warnings.map((w, i) => (
                <div key={i} className="bg-yellow-900/30 border border-yellow-700 text-yellow-200 px-3 py-2 rounded text-sm">
                  {w.message}
                </div>
              ))}
            </div>
          )}

          {/* Holdings Table */}
          <table className="w-full text-sm">
            <thead>
              <tr className="border-b border-gray-700 text-gray-400">
                <th className="text-left py-2">Ticker</th>
                <th className="text-left py-2">Name</th>
                <th className="text-right py-2">Weight</th>
                <th className="text-right py-2">Yield</th>
                <th className="text-right py-2">Beta</th>
              </tr>
            </thead>
            <tbody>
              {currentResult.portfolio.holdings.map((h) => (
                <tr key={h.ticker} className="border-b border-gray-700/50">
                  <td className="py-2 font-mono font-medium text-primary-300">{h.ticker}</td>
                  <td className="py-2 text-gray-300">{h.name}</td>
                  <td className="py-2 text-right">{h.weight.toFixed(1)}%</td>
                  <td className="py-2 text-right text-green-400">{h.yield.toFixed(2)}%</td>
                  <td className="py-2 text-right text-blue-400">{h.beta.toFixed(3)}</td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      )}
    </div>
  );
}