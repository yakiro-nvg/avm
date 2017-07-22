project(avm)

include_directories(${AVM_ROOT_DIRECTORY}/src/inc)

file(GLOB_RECURSE PUB_HEADERS ${AVM_ROOT_DIRECTORY}/src/inc/*.h)
file(GLOB_RECURSE HEADERS 	  ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE SOURCES 	  ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(avm SHARED ${PUB_HEADERS} ${HEADERS} ${SOURCES})

add_definitions(-DANY_EXPORT -DANY_SHARED)