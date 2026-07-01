export function BackTestPage() {
  return (
    <div className="max-w-4xl">
      <h2 className="text-2xl font-bold mb-6">Back-Test</h2>
      <div className="bg-gray-800 rounded-lg p-6 text-gray-400">
        <p>Select a portfolio to back-test its historical performance.</p>
        <p className="mt-2 text-sm">Configure date range, rebalancing frequency, and benchmark.</p>
      </div>
    </div>
  );
}