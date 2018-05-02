option(AVM_ENABLE_UTEST "Build unit testing" OFF)

add_subdirectory(private)

if(AVM_ENABLE_UTEST)
    add_subdirectory(utest)
endif()