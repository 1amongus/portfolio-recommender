cmake_minimum_required(VERSION 3.21)
project(PortfolioRecommender VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick QuickControls2 Network Charts Widgets)

enable_testing()

add_subdirectory(src)
add_subdirectory(tests)

install(TARGETS PortfolioRecommenderApp
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

include(GNUInstallDirs)
if(WIN32)
    find_program(WINDEPLOYQT windeployqt HINTS ${CMAKE_PREFIX_PATH} PATH_SUFFIXES bin)
    if(WINDEPLOYQT)
        install(CODE "
            execute_process(
                COMMAND \"${WINDEPLOYQT}\" --qmldir \"${CMAKE_SOURCE_DIR}/src/ui/qml\" \"\${CMAKE_INSTALL_PREFIX}/bin/PortfolioRecommenderApp.exe\"
            )
        ")
    endif()
endif()

add_subdirectory(packaging)