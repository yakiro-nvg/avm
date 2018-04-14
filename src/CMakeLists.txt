project(avm C)

include_directories(${AVM_ROOT_DIRECTORY}/src/inc)

configure_file(config.h.in ${AVM_ROOT_DIRECTORY}/src/inc/avm/config.h)

set(STACK_TYPES value)
foreach(STACK_TYPE ${STACK_TYPES})
    string(TOUPPER ${STACK_TYPE} STACK_TYPE_UPPER)
    configure_file(private/stack.h.in ${CMAKE_CURRENT_SOURCE_DIR}/private/${STACK_TYPE}_stack.h)
    configure_file(private/stack.c.in ${CMAKE_CURRENT_SOURCE_DIR}/private/${STACK_TYPE}_stack.c)
endforeach()

file(GLOB_RECURSE API_HEADERS ${AVM_ROOT_DIRECTORY}/src/inc/*.h)
file(GLOB_RECURSE HEADERS 	  private/*.h)
file(GLOB_RECURSE SOURCES 	  private/*.c)

add_library(avm SHARED
	${API_HEADERS} ${HEADERS} ${SOURCES})
add_definitions(-DAEXPORT)