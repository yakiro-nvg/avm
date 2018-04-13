project(avm C)

include_directories(${AVM_ROOT_DIRECTORY}/src/inc)

configure_file(config.h.in ${AVM_ROOT_DIRECTORY}/src/inc/avm/config.h)

file(GLOB_RECURSE API_HEADERS ${AVM_ROOT_DIRECTORY}/src/inc/*.h)
file(GLOB_RECURSE HEADERS 	  private/*.h)
file(GLOB_RECURSE SOURCES 	  private/*.c)

add_library(avm SHARED
	${API_HEADERS} ${HEADERS} ${SOURCES})
add_definitions(-DAEXPORT)