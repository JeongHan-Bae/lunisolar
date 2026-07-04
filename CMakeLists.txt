cmake_minimum_required(VERSION 4.1)

project(lunisolar LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

message(STATUS "lunisolar C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "lunisolar build type: ${CMAKE_BUILD_TYPE}")

add_executable(test_lunisolar
        tests/test_lunisolar.cpp
)

target_compile_features(test_lunisolar PRIVATE cxx_std_20)

target_include_directories(test_lunisolar PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

enable_testing()
add_test(NAME test_lunisolar COMMAND test_lunisolar)

add_custom_target(run_test_lunisolar
        COMMAND $<TARGET_FILE:test_lunisolar>
        DEPENDS test_lunisolar
        USES_TERMINAL
)
