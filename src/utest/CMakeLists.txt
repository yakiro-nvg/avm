project(utest)

include_directories(${AVM_ROOT_DIRECTORY}/src/utest)
include_directories(${AVM_ROOT_DIRECTORY}/src/inc)
include_directories(${CATCH_INCLUDE_DIR})

file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(utest ${HEADERS} ${SOURCES})
add_sanitizers(utest)

target_link_libraries(utest avm)
add_dependencies(utest catch)
add_test(NAME utest COMMAND utest)

if(CHECK_COVERAGE)
    set(COVERAGE_EXCLUDES '/usr/*' '*/catch/*')
    setup_target_for_coverage(NAME coverage EXECUTABLE ctest DEPENDENCIES utest)
endif()
