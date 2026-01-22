cmake_minimum_required(VERSION 3.20)

project(concurrency_playground LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(googletest)


enable_testing()

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

include(GoogleTest)

add_compile_options("-Wall;-Wextra;-Werror")

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(benchmark)