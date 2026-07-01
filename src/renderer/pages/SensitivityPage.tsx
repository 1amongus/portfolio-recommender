export function SensitivityPage() {
  return (
    <div className="max-w-4xl">
      <h2 className="text-2xl font-bold mb-6">Yield & Beta Sensitivity</h2>
      <div className="bg-gray-800 rounded-lg p-6 text-gray-400">
        <p>Generate a sensitivity curve showing how portfolio Beta changes as yield targets vary.</p>
        <p className="mt-2 text-sm">Configure yield range and step size to explore the trade-off frontier.</p>
      </div>
    </div>
  );
}