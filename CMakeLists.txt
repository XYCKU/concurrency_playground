cmake_minimum_required(VERSION 3.20)
project(concurrency_playground
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options("-Wall;-Wextra;-Werror")

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(benchmark)