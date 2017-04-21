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

int runAllTests(int argc, const char* argv[])
{
    return Catch::Session().run(argc, argv);
}

int main(int argc, const char* argv[])
{
    srand((unsigned int)time(NULL));
    return runAllTests(argc, argv);
}