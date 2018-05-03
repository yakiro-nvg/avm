project(avm)

option(AVM_BUILD_STATIC_LIBS "Build the static library" OFF)

include_directories(${AVM_ROOT_DIRECTORY}/src/inc)

if(AVM_BUILD_STATIC_LIBS)
    set(LINK_TYPE STATIC)
else()
    set(LINK_TYPE SHARED)
    add_definitions(-DAEXPORT)
endif()
configure_file(config.h.in ${AVM_ROOT_DIRECTORY}/src/inc/avm/config.h)

file(GLOB_RECURSE API_HEADERS ${AVM_ROOT_DIRECTORY}/src/inc/*.h)
file(GLOB_RECURSE HEADERS *.h)
file(GLOB_RECURSE SOURCES *.c)

add_library(avm ${LINK_TYPE} ${API_HEADERS} ${HEADERS} ${SOURCES})