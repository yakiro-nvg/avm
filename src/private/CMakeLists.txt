project(avm C)

option(BUILD_STATIC_LIBS "Build the static library" ON)

include_directories(${AVM_ROOT_DIRECTORY}/src/inc)

if(BUILD_STATIC_LIBS)
    set(LINK_TYPE STATIC)
else()
    set(LINK_TYPE SHARED)
    add_definitions(-DAEXPORT)
endif()
configure_file(config.h.in ${AVM_ROOT_DIRECTORY}/src/inc/avm/config.h)

function(def_stack TYPE_SIMPLE_NAME TYPE_NAME TYPE_HEADER)
    string(TOUPPER ${TYPE_SIMPLE_NAME} TYPE_UPPER)
    configure_file(
        stack.h.in
        ${CMAKE_CURRENT_SOURCE_DIR}/stacks/${TYPE_SIMPLE_NAME}_stack.h)
    configure_file(
        stack.c.in
        ${CMAKE_CURRENT_SOURCE_DIR}/stacks/${TYPE_SIMPLE_NAME}_stack.c)
endfunction()

def_stack(s32         s32            avm/prereq.h)
def_stack(value       avalue_t       ../value.h)
def_stack(instruction ainstruction_t ../opcode.h)
def_stack(constant    aconstant_t    ../chunk.h)
def_stack(import      aimport_t      ../chunk.h)

file(GLOB_RECURSE API_HEADERS ${AVM_ROOT_DIRECTORY}/src/inc/*.h)
file(GLOB_RECURSE HEADERS private/*.h)
file(GLOB_RECURSE SOURCES private/*.c)

add_library(avm ${LINK_TYPE} ${API_HEADERS} ${HEADERS} ${SOURCES})