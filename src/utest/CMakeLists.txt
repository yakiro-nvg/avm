include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${AVM_ROOT_DIRECTORY}/externals/catch2/single_include)
include_directories(${AVM_ROOT_DIRECTORY}/src/inc)
include_directories(${AVM_ROOT_DIRECTORY}/src/private)

file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(utest ${HEADERS} ${SOURCES})
target_link_libraries(utest avm)
include(ParseAndAddCatchTests)
ParseAndAddCatchTests(utest)
set_property(TARGET utest PROPERTY CXX_STANDARD 11)