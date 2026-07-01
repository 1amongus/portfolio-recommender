import { BrowserRouter, Routes, Route, NavLink } from 'react-router-dom';
import { GeneratePage } from './pages/GeneratePage.js';
import { BackTestPage } from './pages/BackTestPage.js';
import { SensitivityPage } from './pages/SensitivityPage.js';
import { ImportPage } from './pages/ImportPage.js';
import { SavedPage } from './pages/SavedPage.js';
import { SettingsPage } from './pages/SettingsPage.js';

const navItems = [
  { to: '/', label: 'Generate' },
  { to: '/backtest', label: 'Back-Test' },
  { to: '/sensitivity', label: 'Sensitivity' },
  { to: '/import', label: 'Import' },
  { to: '/saved', label: 'Saved' },
  { to: '/settings', label: 'Settings' },
];

export function App() {
  return (
    <BrowserRouter>
      <div className="flex h-screen">
        {/* Sidebar */}
        <nav className="w-56 bg-gray-800 border-r border-gray-700 flex flex-col p-4 gap-1">
          <h1 className="text-lg font-bold text-primary-400 mb-6 px-3">Portfolio Recommender</h1>
          {navItems.map((item) => (
            <NavLink
              key={item.to}
              to={item.to}
              className={({ isActive }) =>
                `px-3 py-2 rounded-md text-sm font-medium transition-colors ${
                  isActive
                    ? 'bg-primary-600 text-white'
                    : 'text-gray-300 hover:bg-gray-700 hover:text-white'
                }`
              }
            >
              {item.label}
            </NavLink>
          ))}
        </nav>

        {/* Main Content */}
        <main className="flex-1 overflow-auto p-6">
          <Routes>
            <Route path="/" element={<GeneratePage />} />
            <Route path="/backtest" element={<BackTestPage />} />
            <Route path="/sensitivity" element={<SensitivityPage />} />
            <Route path="/import" element={<ImportPage />} />
            <Route path="/saved" element={<SavedPage />} />
            <Route path="/settings" element={<SettingsPage />} />
          </Routes>
        </main>
      </div>
    </BrowserRouter>
  );
}