"""Default asset universe - 20 curated dividend-paying stocks and ETFs."""

from datetime import datetime
from ..models import Asset

DEFAULT_UNIVERSE = [
    Asset("SCHD", "Schwab U.S. Dividend Equity ETF", "ETF", 79.0, 0.036, 0.88, 50e9, True, datetime.utcnow()),
    Asset("VYM", "Vanguard High Dividend Yield ETF", "ETF", 118.0, 0.031, 0.85, 54e9, True, datetime.utcnow()),
    Asset("KO", "Coca-Cola", "Consumer Defensive", 63.0, 0.029, 0.58, 270e9, False, datetime.utcnow()),
    Asset("JNJ", "Johnson & Johnson", "Healthcare", 148.0, 0.032, 0.54, 360e9, False, datetime.utcnow()),
    Asset("PG", "Procter & Gamble", "Consumer Defensive", 165.0, 0.025, 0.43, 390e9, False, datetime.utcnow()),
    Asset("XLU", "Utilities Select Sector SPDR Fund", "Utilities", 69.0, 0.034, 0.61, 13e9, True, datetime.utcnow()),
    Asset("O", "Realty Income", "Real Estate", 56.0, 0.054, 0.72, 49e9, False, datetime.utcnow()),
    Asset("T", "AT&T", "Communication Services", 18.0, 0.065, 0.75, 130e9, False, datetime.utcnow()),
    Asset("MO", "Altria", "Consumer Defensive", 45.0, 0.082, 0.63, 78e9, False, datetime.utcnow()),
    Asset("ABBV", "AbbVie", "Healthcare", 171.0, 0.038, 0.68, 300e9, False, datetime.utcnow()),
    Asset("PFE", "Pfizer", "Healthcare", 28.0, 0.057, 0.65, 160e9, False, datetime.utcnow()),
    Asset("VZ", "Verizon", "Communication Services", 40.0, 0.063, 0.42, 170e9, False, datetime.utcnow()),
    Asset("IBM", "IBM", "Technology", 173.0, 0.035, 0.95, 160e9, False, datetime.utcnow()),
    Asset("CVX", "Chevron", "Energy", 156.0, 0.039, 1.12, 290e9, False, datetime.utcnow()),
    Asset("XOM", "ExxonMobil", "Energy", 115.0, 0.033, 0.92, 460e9, False, datetime.utcnow()),
    Asset("HDV", "iShares Core High Dividend ETF", "ETF", 110.0, 0.038, 0.78, 11e9, True, datetime.utcnow()),
    Asset("SPYD", "SPDR Portfolio S&P 500 High Dividend ETF", "ETF", 43.0, 0.045, 0.91, 8e9, True, datetime.utcnow()),
    Asset("DVY", "iShares Select Dividend ETF", "ETF", 124.0, 0.036, 0.82, 19e9, True, datetime.utcnow()),
    Asset("VIG", "Vanguard Dividend Appreciation ETF", "ETF", 188.0, 0.019, 0.88, 82e9, True, datetime.utcnow()),
    Asset("NOBL", "ProShares S&P 500 Dividend Aristocrats ETF", "ETF", 98.0, 0.021, 0.85, 13e9, True, datetime.utcnow()),
]
