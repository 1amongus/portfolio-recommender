export function ImportPage() {
  return (
    <div className="max-w-4xl">
      <h2 className="text-2xl font-bold mb-6">Import Portfolio</h2>
      <div className="bg-gray-800 rounded-lg p-6 text-gray-400">
        <p>Import your existing portfolio from a CSV file or enter holdings manually.</p>
        <p className="mt-2 text-sm">The system will generate a rebalanced version optimized for your target yield.</p>
      </div>
    </div>
  );
}