export function SettingsPage() {
  return (
    <div className="max-w-4xl">
      <h2 className="text-2xl font-bold mb-6">Settings</h2>
      <div className="space-y-6">
        <div className="bg-gray-800 rounded-lg p-6">
          <h3 className="text-lg font-semibold mb-4">API Keys</h3>
          <p className="text-gray-400 text-sm mb-4">
            Configure your API keys for market data providers. Free-tier keys are sufficient for typical usage.
          </p>
          <div className="space-y-3">
            {['Alpha Vantage', 'Financial Modeling Prep', 'Polygon.io', 'FRED'].map((provider) => (
              <div key={provider} className="flex items-center gap-3">
                <label className="w-48 text-sm text-gray-300">{provider}</label>
                <input
                  type="password"
                  placeholder="Enter API key..."
                  className="flex-1 px-3 py-2 bg-gray-700 border border-gray-600 rounded-md text-white text-sm focus:ring-2 focus:ring-primary-500"
                />
                <span className="w-20 text-xs text-gray-500">Not set</span>
              </div>
            ))}
          </div>
        </div>
      </div>
    </div>
  );
}