find_package(Qt6 REQUIRED COMPONENTS Test)

qt_add_executable(OptimizerTests
    unit/tst_optimizer.cpp
    ../src/services/optimization/Optimizer.cpp
    ../src/services/optimization/Optimizer.h
    ../src/services/data/DataStore.cpp
    ../src/services/data/DataStore.h
    ../src/models/Portfolio.h
    ../src/models/Asset.h
    ../src/utils/Math.cpp
    ../src/utils/Math.h
)

target_include_directories(OptimizerTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/../src)

target_link_libraries(OptimizerTests PRIVATE Qt6::Core Qt6::Test)
add_test(NAME OptimizerTests COMMAND OptimizerTests)

qt_add_executable(MathTests
    unit/tst_math.cpp
    ../src/utils/Math.cpp
    ../src/utils/Math.h
    ../src/models/Portfolio.h
)

target_include_directories(MathTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/../src)

target_link_libraries(MathTests PRIVATE Qt6::Core Qt6::Test)
add_test(NAME MathTests COMMAND MathTests)

qt_add_executable(BacktestTests
    unit/tst_backtest.cpp
    ../src/services/backtest/Engine.cpp
    ../src/services/backtest/Engine.h
    ../src/services/data/DataStore.cpp
    ../src/services/data/DataStore.h
    ../src/models/BacktestResult.h
    ../src/models/Portfolio.h
    ../src/models/Asset.h
    ../src/utils/Math.cpp
    ../src/utils/Math.h
)

target_include_directories(BacktestTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/../src)

target_link_libraries(BacktestTests PRIVATE Qt6::Core Qt6::Test)
add_test(NAME BacktestTests COMMAND BacktestTests)

qt_add_executable(CsvParserTests
    unit/tst_csvparser.cpp
    ../src/services/import/CsvParser.cpp
    ../src/services/import/CsvParser.h
    ../src/services/data/DataStore.cpp
    ../src/services/data/DataStore.h
    ../src/models/Portfolio.h
    ../src/models/Asset.h
)

target_include_directories(CsvParserTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/../src)

target_link_libraries(CsvParserTests PRIVATE Qt6::Core Qt6::Test)
add_test(NAME CsvParserTests COMMAND CsvParserTests)
