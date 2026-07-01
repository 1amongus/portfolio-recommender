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