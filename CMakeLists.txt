cmake_minimum_required(VERSION 2.8.7)

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)
set(USE_FOLDERS TRUE)

enable_testing()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(AVM_ROOT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

if(UNIX AND NOT WIN32 AND NOT APPLE)
  find_program(CMAKE_UNAME uname /bin /usr/bin /usr/local/bin)
  if(CMAKE_UNAME)
    exec_program(uname ARGS -m OUTPUT_VARIABLE CMAKE_SYSTEM_PROCESSOR)
    set(CMAKE_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR} CACHE INTERNAL "processor type (i386 and x86_64)")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    	add_definitions(-fPIC)
    endif()
  endif()
endif()

set(CMAKE_C_FLAGS_DEBUG "-DANY_DEBUG")

add_definitions(-DANY_SMP -DANY_TOOL)

add_subdirectory(cmake)
add_subdirectory(src)