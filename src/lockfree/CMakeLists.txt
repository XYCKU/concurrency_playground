add_library(lockfree INTERFACE)

target_include_directories(lockfree INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(lockfree INTERFACE utils)