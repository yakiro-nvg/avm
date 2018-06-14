// Copyright (c) 2017-2018 Giang "Yakiro" Nguyen. All rights reserved.
#define CATCH_CONFIG_RUNNER
#include <catch.hpp>

#include <stdlib.h>
#include <exception>

static int runAllTests(int argc, char *argv[])
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

int main(int argc, char* argv[])
{
    return runAllTests(argc, argv);
}