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

add_executable(unbounded_stack_test UnboundedStack_tests.cpp)
target_link_libraries(unbounded_stack_test PRIVATE lockfree gtest_main)
target_compile_options(unbounded_stack_test PRIVATE -fsanitize=address)
target_link_options(unbounded_stack_test PRIVATE -fsanitize=address)
gtest_discover_tests(unbounded_stack_test)


add_executable(msqueue_test MSQueue_tests.cpp)
target_link_libraries(msqueue_test PRIVATE lockfree gtest_main)
target_compile_options(msqueue_test PRIVATE -fsanitize=thread)
target_link_options(msqueue_test PRIVATE -fsanitize=thread)
gtest_discover_tests(msqueue_test)


add_executable(spscringbuffer_test SPSCRingBuffer_tests.cpp)
target_link_libraries(spscringbuffer_test PRIVATE lockfree gtest_main)
target_compile_options(spscringbuffer_test PRIVATE -fsanitize=thread)
target_link_options(spscringbuffer_test PRIVATE -fsanitize=thread)
gtest_discover_tests(spscringbuffer_test)

add_executable(mpmcringbuffer_test MPMCRingBuffer_tests.cpp)
target_link_libraries(mpmcringbuffer_test PRIVATE lockfree gtest_main)
target_compile_options(mpmcringbuffer_test PRIVATE -fsanitize=thread)
target_link_options(mpmcringbuffer_test PRIVATE -fsanitize=thread)
gtest_discover_tests(mpmcringbuffer_test)

add_executable(blockingringbuffer_test BlockingRingBuffer_tests.cpp)
target_link_libraries(blockingringbuffer_test PRIVATE blocking gtest_main)
target_compile_options(blockingringbuffer_test PRIVATE -fsanitize=thread)
target_link_options(blockingringbuffer_test PRIVATE -fsanitize=thread)
gtest_discover_tests(blockingringbuffer_test)

