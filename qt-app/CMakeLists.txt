cmake_minimum_required(VERSION 3.21)
project(PortfolioRecommender VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick QuickControls2 Network Charts)

enable_testing()

add_subdirectory(src)
add_subdirectory(tests)