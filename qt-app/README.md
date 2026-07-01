# Portfolio Recommender Qt App

## Prerequisites
- Qt 6.7 or newer
- CMake 3.21 or newer
- A C++17-capable compiler

## Build
```powershell
cmake -S . -B build
cmake --build build
```

## Run
```powershell
.\build\src\PortfolioRecommenderApp.exe
```

## Project structure
- `src/app` - application entry point
- `src/models` - domain models
- `src/services` - data access and optimization services
- `src/ui/controllers` - QML-facing controllers
- `src/ui/qml` - QML views and components
- `src/utils` - math and logging helpers
- `tests/unit` - Qt Test unit tests