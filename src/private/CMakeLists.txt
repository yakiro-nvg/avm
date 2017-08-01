project(avm C ASM)

include_directories(${AVM_ROOT_DIRECTORY}/src/inc)

file(GLOB_RECURSE PUB_HEADERS ${AVM_ROOT_DIRECTORY}/src/inc/*.h)
file(GLOB_RECURSE HEADERS 	  ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE SOURCES 	  ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -x assembler-with-cpp")
    set(SOURCES ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/task_gccasm_asm.S)
endif()

add_library(avm SHARED ${PUB_HEADERS} ${HEADERS} ${SOURCES})

add_sanitizers(avm)

add_definitions(-DANY_EXPORT -DANY_SHARED)
