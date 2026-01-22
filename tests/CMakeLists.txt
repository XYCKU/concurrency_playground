function(add_test_target target file)
    add_executable(${target} ${file})
    target_link_libraries(${target} PRIVATE blocking lockfree gtest gtest_main)

    if (ENABLE_ASAN)
        target_compile_options(${target} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(${target} PRIVATE -fsanitize=address)
    endif()

    if (ENABLE_TSAN)
        target_compile_options(${target} PRIVATE -fsanitize=thread)
        target_link_options(${target} PRIVATE -fsanitize=thread)
    endif()

    if (ENABLE_UBSAN)
        target_compile_options(${target} PRIVATE -fsanitize=undefined)
        target_link_options(${target} PRIVATE -fsanitize=undefined)
    endif()

    gtest_discover_tests(${target})
endfunction()

add_test_target(unbounded_stack_test UnboundedStack_tests.cpp)
add_test_target(msqueue_test MSQueue_tests.cpp)
add_test_target(spscringbuffer_test SPSCRingBuffer_tests.cpp)
add_test_target(mpmcringbuffer_test MPMCRingBuffer_tests.cpp)
add_test_target(blockingringbuffer_test BlockingRingBuffer_tests.cpp)

#add_test_target(Ring_twist_test Twist_tests.cpp)