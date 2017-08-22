include_directories(${AVM_ROOT_DIRECTORY}/externals/ProgramOptions.hxx)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${AVM_ROOT_DIRECTORY}/src/inc)

file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(amlc ${HEADERS} ${SOURCES})
add_sanitizers(amlc)

target_link_libraries(amlc avm)
target_compile_features(amlc PRIVATE cxx_lambdas)
