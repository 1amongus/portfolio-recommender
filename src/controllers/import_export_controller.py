"""Import/Export controller - CSV import and export."""

import csv
import io
from pathlib import Path

from PySide6.QtCore import QObject, Signal, Slot, QUrl
from ..models import Asset
from ..services.data_store import DataStore


class ImportExportController(QObject):
    importCompleted = Signal(int)  # number of assets imported
    exportCompleted = Signal(str)  # path exported to
    errorOccurred = Signal(str)

    def __init__(self, data_store: DataStore, parent=None):
        super().__init__(parent)
        self._data_store = data_store

    @Slot(str)
    def importCsv(self, file_url: str):
        """Import assets from a CSV file."""
        try:
            path = QUrl(file_url).toLocalFile() if file_url.startswith("file") else file_url
            filepath = Path(path)
            if not filepath.exists():
                self.errorOccurred.emit(f"File not found: {path}")
                return

            with open(filepath, "r", encoding="utf-8") as f:
                reader = csv.DictReader(f)
                new_assets = []
                for row in reader:
                    ticker = row.get("ticker", row.get("Ticker", "")).strip().upper()
                    if not ticker:
                        continue
                    asset = Asset(
                        ticker=ticker,
                        name=row.get("name", row.get("Name", "")),
                        sector=row.get("sector", row.get("Sector", "")),
                        dividend_yield=float(row.get("dividendYield", row.get("yield", row.get("Yield", 0)))),
                        beta=float(row.get("beta", row.get("Beta", 0))),
                        price=float(row.get("price", row.get("Price", 0))),
                    )
                    new_assets.append(asset)

            if not new_assets:
                self.errorOccurred.emit("No valid assets found in CSV.")
                return

            # Merge with existing
            existing = self._data_store.load_assets()
            existing_tickers = {a.ticker.upper() for a in existing}
            added = 0
            for asset in new_assets:
                if asset.ticker not in existing_tickers:
                    existing.append(asset)
                    added += 1

            self._data_store.save_assets(existing)
            self.importCompleted.emit(added)
            print(f"[ImportExport] Imported {added} new assets from {filepath.name}")

        except Exception as e:
            self.errorOccurred.emit(str(e))

    @Slot(str, "QVariant")
    def exportCsv(self, file_url: str, holdings_data):
        """Export portfolio holdings to CSV."""
        try:
            path = QUrl(file_url).toLocalFile() if file_url.startswith("file") else file_url
            holdings = holdings_data if isinstance(holdings_data, list) else holdings_data.toVariant()
            if not holdings:
                self.errorOccurred.emit("No holdings to export.")
                return

            with open(path, "w", newline="", encoding="utf-8") as f:
                writer = csv.writer(f)
                writer.writerow(["Ticker", "Weight (%)", "Yield (%)", "Beta"])
                for h in holdings:
                    writer.writerow([
                        h.get("ticker", ""),
                        f"{h.get('weight', 0) * 100:.2f}",
                        f"{h.get('yield', 0) * 100:.2f}",
                        f"{h.get('beta', 0):.3f}",
                    ])

            self.exportCompleted.emit(path)
            print(f"[ImportExport] Exported {len(holdings)} holdings to {path}")

        except Exception as e:
            self.errorOccurred.emit(str(e))
