set(CMAKE_BUILD_TYPE Release)

include(FetchContent)
FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.9.4
)

FetchContent_MakeAvailable(benchmark)

add_executable(RingBuffer_bench RingBuffer_bench.cpp)
target_compile_options(RingBuffer_bench PRIVATE -O3 -DNDEBUG)
target_link_libraries(RingBuffer_bench PRIVATE benchmark::benchmark blocking lockfree)