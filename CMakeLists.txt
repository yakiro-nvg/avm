cmake_minimum_required(VERSION 2.8.7)

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)
set(USE_FOLDERS TRUE)

enable_testing()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(AVM_ROOT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DANY_DEBUG")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/externals/sanitizers-cmake/cmake" ${CMAKE_MODULE_PATH})
find_package(Sanitizers REQUIRED)

option(USE_VALGRIND "Enable Valgrid Support." Off)
if (USE_VALGRIND)
    add_definitions(-DANY_USE_VALGRIND)
endif()

add_definitions(-DANY_SMP -DANY_TOOL)

add_subdirectory(cmake)
add_subdirectory(src)
