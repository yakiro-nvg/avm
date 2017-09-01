cmake_minimum_required(VERSION 2.8.7)

project(Any-VM)

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)
set(USE_FOLDERS TRUE)

enable_testing()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FindPackageHandleStandardArgs)

set(AVM_ROOT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DANY_DEBUG")

set(CMAKE_MODULE_PATH 
	"${CMAKE_CURRENT_SOURCE_DIR}/externals/sanitizers-cmake/cmake"
	${CMAKE_MODULE_PATH})
find_package(Sanitizers REQUIRED)

option(USE_VALGRIND "Enable Valgrid Support." Off)
if (USE_VALGRIND)
    find_package(VALGRIND REQUIRED)
    add_definitions(-DANY_USE_VALGRIND)
endif()

set(TASK_BACKEND "UNDEFINED" CACHE STRING "")
if(${TASK_BACKEND} MATCHES "fiber")
	set(TASK_BACKEND_STRING "FIBER")
elseif(${TASK_BACKEND} MATCHES "gccasm")
	set(TASK_BACKEND_STRING "GCCASM")
else()
    message(FATAL_ERROR "Unknown task backend ${TASK_BACKEND}")
endif()

option(CHECK_COVERAGE "Enable Coverage Checking." Off)
if(CHECK_COVERAGE)
    include(CodeCoverage)
    APPEND_COVERAGE_COMPILER_FLAGS()
endif()

option(AMLC "Enable Any Machine Language Compiler." Off)
if(AMLC)
    add_subdirectory(tools/amlc)
endif()

add_subdirectory(cmake)
add_subdirectory(src)