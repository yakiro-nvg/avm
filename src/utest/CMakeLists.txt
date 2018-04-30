option(AVM_ENABLE_COVERAGE "Generate coverage for codecov.io" OFF)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${AVM_ROOT_DIRECTORY}/externals/catch2/single_include)
include_directories(${AVM_ROOT_DIRECTORY}/src/inc)
include_directories(${AVM_ROOT_DIRECTORY}/src/private)

file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(utest ${HEADERS} ${SOURCES})
target_link_libraries(utest avm rapidcheck)
include(ParseAndAddCatchTests)
ParseAndAddCatchTests(utest)
set_property(TARGET utest PROPERTY CXX_STANDARD 11)

if (AVM_ENABLE_COVERAGE)
    set(ENABLE_COVERAGE ON CACHE BOOL "Enable coverage build." FORCE)
    find_package(codecov)
    add_coverage(utest)
    list(APPEND LCOV_REMOVE_PATTERNS "'/usr/*'")
    list(APPEND LCOV_REMOVE_PATTERNS "'*/rapidcheck/*'")
    coverage_evaluate()
endif()