from dataclasses import dataclass, field
from datetime import datetime
from typing import List
import uuid

from .holding import Holding


@dataclass
class Portfolio:
    id: str = field(default_factory=lambda: str(uuid.uuid4()))
    name: str = "Generated Portfolio"
    target_yield: float = 0.0
    achieved_yield: float = 0.0
    aggregate_beta: float = 0.0
    holdings: List[Holding] = field(default_factory=list)
    created_at: datetime = field(default_factory=datetime.utcnow)
