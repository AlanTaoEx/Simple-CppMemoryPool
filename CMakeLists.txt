cmake_minimum_required(VERSION 3.10.0)
project(MemoryPool VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall")
set(CMAKE_BUILD_TYPE Debug)

add_executable(MemoryPoolTest benchmark.cpp)

target_include_directories(MemoryPoolTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

enable_testing()
add_test(NAME MemoryPoolTest COMMAND MemoryPoolTest)

