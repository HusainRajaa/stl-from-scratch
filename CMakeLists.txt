cmake_minimum_required(VERSION 3.20)
project(stl_from_scratch VERSION 1.0.0 LANGUAGES CXX)

# Target C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optimization and warning flags
if(MSVC)
    add_compile_options(/W4 /O2)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O3)
endif()

# Header-only library interface
add_library(stl_headers INTERFACE)
target_include_directories(stl_headers INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

enable_testing()

# Test runner executable
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_main.cpp")
    add_executable(stl_unit_tests tests/test_main.cpp)
    target_link_libraries(stl_unit_tests PRIVATE stl_headers)
    target_include_directories(stl_unit_tests PRIVATE tests)
    add_test(NAME stl_unit_tests COMMAND stl_unit_tests)
endif()

# Separate test targets if present
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "tests/test_*.cpp")
foreach(TEST_SRC ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)
    if(NOT "${TEST_NAME}" STREQUAL "test_main")
        add_executable(${TEST_NAME} ${TEST_SRC})
        target_link_libraries(${TEST_NAME} PRIVATE stl_headers)
        target_include_directories(${TEST_NAME} PRIVATE tests)
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    endif()
endforeach()

file(GLOB DEMO_SOURCES CONFIGURE_DEPENDS "src/demo_*.cpp")
foreach(DEMO_SRC ${DEMO_SOURCES})
    get_filename_component(DEMO_NAME ${DEMO_SRC} NAME_WE)
    add_executable(${DEMO_NAME} ${DEMO_SRC})
    target_link_libraries(${DEMO_NAME} PRIVATE stl_headers)
    add_test(NAME ${DEMO_NAME} COMMAND ${DEMO_NAME})
endforeach()

# Benchmarks
file(GLOB BENCH_SOURCES CONFIGURE_DEPENDS "benchmarks/benchmark_*.cpp")
foreach(BENCH_SRC ${BENCH_SOURCES})
    get_filename_component(BENCH_NAME ${BENCH_SRC} NAME_WE)
    add_executable(${BENCH_NAME} ${BENCH_SRC})
    target_link_libraries(${BENCH_NAME} PRIVATE stl_headers)
    target_include_directories(${BENCH_NAME} PRIVATE tests benchmarks)
endforeach()
