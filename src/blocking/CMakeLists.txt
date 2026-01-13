add_library(blocking INTERFACE)

target_include_directories(blocking INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(blocking INTERFACE utils)