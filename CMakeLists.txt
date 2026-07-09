cmake_minimum_required(VERSION 3.12)

project(lunisolar LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

message(STATUS "lunisolar C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "lunisolar build type: ${CMAKE_BUILD_TYPE}")

add_executable(test_lunisolar
        tests/test_lunisolar.cpp
)

add_executable(example_birthdays
        examples/birthdays.cpp
)

add_executable(example_solar_terms
        examples/solar_terms.cpp
)

target_compile_features(test_lunisolar PRIVATE cxx_std_20)
target_compile_features(example_birthdays PRIVATE cxx_std_20)
target_compile_features(example_solar_terms PRIVATE cxx_std_20)

target_include_directories(test_lunisolar PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

target_include_directories(example_birthdays PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(example_solar_terms PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

enable_testing()
add_test(NAME test_lunisolar COMMAND test_lunisolar)

add_custom_target(run_test_lunisolar
        COMMAND $<TARGET_FILE:test_lunisolar>
        DEPENDS test_lunisolar
        USES_TERMINAL
)

add_custom_target(run_example_birthdays
        COMMAND $<TARGET_FILE:example_birthdays>
        DEPENDS example_birthdays
        USES_TERMINAL
)

add_custom_target(run_example_solar_terms
        COMMAND $<TARGET_FILE:example_solar_terms>
        DEPENDS example_solar_terms
        USES_TERMINAL
)
