project(avm C)

include_directories(${AVM_ROOT_DIRECTORY}/src/inc)

configure_file(config.h.in ${AVM_ROOT_DIRECTORY}/src/inc/avm/config.h)

function(def_stack TYPE_SIMPLE_NAME TYPE_NAME TYPE_HEADER)
    string(TOUPPER ${TYPE_SIMPLE_NAME} TYPE_UPPER)
    configure_file(
        private/stack.h.in
        ${CMAKE_CURRENT_SOURCE_DIR}/private/stacks/${TYPE_SIMPLE_NAME}.h)
    configure_file(
        private/stack.c.in
        ${CMAKE_CURRENT_SOURCE_DIR}/private/stacks/${TYPE_SIMPLE_NAME}.c)
endfunction()

def_stack(s32         s32            avm/prereq.h)
def_stack(value       avalue_t       ../value.h)
def_stack(instruction ainstruction_t ../opcode.h)
def_stack(constant    aconstant_t    ../chunk.h)
def_stack(import      aimport_t      ../chunk.h)

file(GLOB_RECURSE API_HEADERS ${AVM_ROOT_DIRECTORY}/src/inc/*.h)
file(GLOB_RECURSE HEADERS private/*.h)
file(GLOB_RECURSE SOURCES private/*.c)

add_library(avm SHARED
	${API_HEADERS} ${HEADERS} ${SOURCES})
add_definitions(-DAEXPORT)