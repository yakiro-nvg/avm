/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/platform.h>
#define CATCH_CONFIG_RUNNER
#ifdef AMSVC
#pragma warning(push)
#pragma warning(disable: 4702) // unreachable code
#endif
#include <catch.hpp>
#ifdef AMSVC
#pragma warning(pop)
#endif

#include <time.h>
#include <stdlib.h>
#include <exception>

int runAllTests(int argc, const char* argv[])
{
    int r = 0;
    try {
        r = Catch::Session().run(argc, argv);
    } catch (std::exception& e) {
        printf("unexpected exception %s\n", e.what());
        r = 1;
    }
    return r;
}

int main(int argc, const char* argv[])
{
    srand((unsigned int)time(NULL));
    return runAllTests(argc, argv);
}