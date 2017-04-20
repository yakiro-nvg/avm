/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#define CATCH_CONFIG_RUNNER
#include <catch.hpp>

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