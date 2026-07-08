"""AI Assistant controller using Ollama for interactive portfolio guidance."""

import json
from pathlib import Path

from PySide6.QtCore import QObject, Property, Signal, Slot, QUrl, QSettings, QStandardPaths
from PySide6.QtNetwork import QNetworkAccessManager, QNetworkRequest, QNetworkReply


SYSTEM_PROMPT = """You are a friendly portfolio advisor assistant inside a desktop app called "Portfolio Recommender". 
Your job is to guide users through building an optimal dividend portfolio.

The app has these pages:
1. **Generate** - Set a target dividend yield (%) and generate an optimized low-beta portfolio
2. **Backtest** - Simulate historical performance of the generated portfolio
3. **Sensitivity** - See how yield/beta trade-off changes across different targets
4. **Import/Export** - Import assets from CSV or export portfolio
5. **Saved** - View and manage previously generated portfolios
6. **Settings** - Configure API keys and manage asset universe

Key concepts:
- Target Yield: The annual dividend yield the user wants (e.g., 3% = moderate, 5% = aggressive)
- Beta: Measures volatility vs market. Lower is safer (<1.0 = less volatile than market)
- The optimizer picks the lowest-beta combination of stocks that achieves the target yield

Be concise (2-3 sentences max per response). Guide users step by step. 
Suggest specific actions like "Try setting your target yield to 4% and click Generate."
If they're unsure, ask about their risk tolerance and income goals."""


def _harness_dir() -> Path:
    path = Path(QStandardPaths.writableLocation(QStandardPaths.StandardLocation.AppDataLocation)) / "harness"
    path.mkdir(parents=True, exist_ok=True)
    return path


class AssistantController(QObject):
    responseChanged = Signal()
    isThinkingChanged = Signal()
    historyChanged = Signal()
    modelsChanged = Signal()
    settingsChanged = Signal()
    harnessFilesChanged = Signal()

    def __init__(self, parent=None):
        super().__init__(parent)
        self._is_thinking = False
        self._history = []
        self._current_response = ""
        self._network = QNetworkAccessManager(self)
        self._available_models = []
        self._harness_files = []

        # Load persisted settings
        self._settings = QSettings()
        self._model = self._settings.value("slm/model", "phi3:mini")
        self._temperature = float(self._settings.value("slm/temperature", 0.7))
        self._max_tokens = int(self._settings.value("slm/maxTokens", 256))
        self._top_p = float(self._settings.value("slm/topP", 0.9))
        self._base_url = self._settings.value("slm/baseUrl", "http://localhost:11434")

        self._refresh_harness_files()

    # --- Properties ---
    def _get_is_thinking(self) -> bool:
        return self._is_thinking

    isThinking = Property(bool, _get_is_thinking, notify=isThinkingChanged)

    def _get_history(self) -> list:
        return self._history

    history = Property("QVariant", _get_history, notify=historyChanged)

    def _get_current_response(self) -> str:
        return self._current_response

    currentResponse = Property(str, _get_current_response, notify=responseChanged)

    def _get_available_models(self) -> list:
        return self._available_models

    availableModels = Property("QVariant", _get_available_models, notify=modelsChanged)

    def _get_model(self) -> str:
        return self._model

    model = Property(str, _get_model, notify=settingsChanged)

    def _get_temperature(self) -> float:
        return self._temperature

    temperature = Property(float, _get_temperature, notify=settingsChanged)

    def _get_max_tokens(self) -> int:
        return self._max_tokens

    maxTokens = Property(int, _get_max_tokens, notify=settingsChanged)

    def _get_top_p(self) -> float:
        return self._top_p

    topP = Property(float, _get_top_p, notify=settingsChanged)

    def _get_base_url(self) -> str:
        return self._base_url

    baseUrl = Property(str, _get_base_url, notify=settingsChanged)

    def _get_harness_files(self) -> list:
        return self._harness_files

    harnessFiles = Property("QVariant", _get_harness_files, notify=harnessFilesChanged)

    # --- Settings Slots ---
    @Slot()
    def fetchModels(self):
        """Fetch available models from Ollama."""
        request = QNetworkRequest(QUrl(f"{self._base_url}/api/tags"))
        reply = self._network.get(request)
        reply.finished.connect(lambda: self._handle_models_response(reply))

    def _handle_models_response(self, reply: QNetworkReply):
        if reply.error() == QNetworkReply.NetworkError.NoError:
            data = json.loads(reply.readAll().data().decode("utf-8"))
            models = data.get("models", [])
            self._available_models = [
                {"name": m["name"], "size": f"{m.get('size', 0) / 1e9:.1f} GB"}
                for m in models
            ]
        else:
            self._available_models = [{"name": "Cannot reach Ollama", "size": ""}]
        self.modelsChanged.emit()
        reply.deleteLater()

    @Slot(str)
    def setModel(self, model_name: str):
        self._model = model_name
        self._settings.setValue("slm/model", model_name)
        self.settingsChanged.emit()
        print(f"[Assistant] Model set to: {model_name}")

    @Slot(float)
    def setTemperature(self, value: float):
        self._temperature = max(0.0, min(2.0, value))
        self._settings.setValue("slm/temperature", self._temperature)
        self.settingsChanged.emit()

    @Slot(int)
    def setMaxTokens(self, value: int):
        self._max_tokens = max(32, min(4096, value))
        self._settings.setValue("slm/maxTokens", self._max_tokens)
        self.settingsChanged.emit()

    @Slot(float)
    def setTopP(self, value: float):
        self._top_p = max(0.0, min(1.0, value))
        self._settings.setValue("slm/topP", self._top_p)
        self.settingsChanged.emit()

    @Slot(str)
    def setBaseUrl(self, value: str):
        self._base_url = value
        self._settings.setValue("slm/baseUrl", value)
        self.settingsChanged.emit()

    # --- Harness file management ---
    @Slot(str)
    def addHarnessFile(self, file_url: str):
        """Copy an MD file into the harness directory."""
        source = QUrl(file_url).toLocalFile() if file_url.startswith("file") else file_url
        source_path = Path(source)
        if not source_path.exists() or source_path.suffix.lower() != ".md":
            return

        dest = _harness_dir() / source_path.name
        counter = 1
        while dest.exists():
            dest = _harness_dir() / f"{source_path.stem}_{counter}.md"
            counter += 1

        dest.write_text(source_path.read_text(encoding="utf-8"), encoding="utf-8")
        self._refresh_harness_files()
        print(f"[Assistant] Added harness file: {dest.name}")

    @Slot(str)
    def removeHarnessFile(self, filename: str):
        """Remove a harness file."""
        path = _harness_dir() / filename
        if path.exists():
            path.unlink()
        self._refresh_harness_files()
        print(f"[Assistant] Removed harness file: {filename}")

    @Slot(str, result=str)
    def getHarnessContent(self, filename: str) -> str:
        """Read content of a harness file."""
        path = _harness_dir() / filename
        if path.exists():
            return path.read_text(encoding="utf-8")
        return ""

    @Slot()
    def refreshHarnessFiles(self):
        self._refresh_harness_files()

    def _refresh_harness_files(self):
        harness_dir = _harness_dir()
        self._harness_files = []
        for f in sorted(harness_dir.glob("*.md")):
            self._harness_files.append({
                "name": f.name,
                "size": f"{f.stat().st_size / 1024:.1f} KB",
                "path": str(f),
            })
        self.harnessFilesChanged.emit()

    def _build_system_prompt(self) -> str:
        """Build system prompt including harness file content."""
        parts = [SYSTEM_PROMPT]
        harness_dir = _harness_dir()
        for f in sorted(harness_dir.glob("*.md")):
            try:
                content = f.read_text(encoding="utf-8")
                parts.append(f"\n\n--- Context from {f.name} ---\n{content}")
            except Exception:
                pass
        return "\n".join(parts)

    # --- Chat Slots ---
    @Slot(str)
    def sendMessage(self, user_message: str):
        """Send a message to the assistant."""
        if self._is_thinking or not user_message.strip():
            return

        self._history.append({"role": "user", "content": user_message})
        self.historyChanged.emit()

        self._is_thinking = True
        self.isThinkingChanged.emit()
        self._current_response = ""
        self.responseChanged.emit()

        system_prompt = self._build_system_prompt()
        messages = [{"role": "system", "content": system_prompt}]
        for msg in self._history[-10:]:
            messages.append({"role": msg["role"], "content": msg["content"]})

        payload = json.dumps({
            "model": self._model,
            "messages": messages,
            "stream": False,
            "options": {
                "temperature": self._temperature,
                "num_predict": self._max_tokens,
                "top_p": self._top_p,
            }
        }).encode("utf-8")

        request = QNetworkRequest(QUrl(f"{self._base_url}/api/chat"))
        request.setHeader(QNetworkRequest.KnownHeaders.ContentTypeHeader, "application/json")

        reply = self._network.post(request, payload)
        reply.finished.connect(lambda: self._handle_response(reply))

    def _handle_response(self, reply: QNetworkReply):
        if reply.error() != QNetworkReply.NetworkError.NoError:
            error_msg = reply.errorString()
            self._current_response = f"⚠️ Couldn't reach Ollama: {error_msg}\n\nMake sure Ollama is running (`ollama serve`)."
            self._history.append({"role": "assistant", "content": self._current_response})
        else:
            data = json.loads(reply.readAll().data().decode("utf-8"))
            content = data.get("message", {}).get("content", "I'm not sure how to help with that.")
            self._current_response = content
            self._history.append({"role": "assistant", "content": content})

        self.responseChanged.emit()
        self.historyChanged.emit()
        self._is_thinking = False
        self.isThinkingChanged.emit()
        reply.deleteLater()

    @Slot()
    def greet(self):
        """Send initial greeting."""
        if not self._history:
            self._history.append({
                "role": "assistant",
                "content": "\ud83d\udc4b Welcome! I'm your portfolio advisor. What's your investment goal \u2014 steady income, growth, or a mix? I'll help you build the right portfolio."
            })
            self.historyChanged.emit()

    @Slot(str)
    def suggestForPage(self, page_name: str):
        """Provide contextual suggestion when user navigates to a page."""
        suggestions = {
            "Generate": "Set your target dividend yield and click Generate. For moderate income, try 3-4%. For aggressive income, try 5-7%.",
            "Backtest": "Run a backtest to see how your portfolio would have performed historically. Try 12-24 months.",
            "Sensitivity": "This shows the yield vs beta trade-off. Compute the curve to find your optimal risk/reward point.",
            "Import": "You can import additional assets from a CSV file to expand your investment universe.",
            "Saved": "Here are your previously generated portfolios. Load one to view or backtest it.",
            "Settings": "Configure your API key for live market data, or manage which assets are in your universe.",
        }
        hint = suggestions.get(page_name, "")
        if hint:
            self._current_response = f"\ud83d\udca1 {hint}"
            self.responseChanged.emit()

    @Slot()
    def clearHistory(self):
        self._history = []
        self._current_response = ""
        self.historyChanged.emit()
        self.responseChanged.emit()
