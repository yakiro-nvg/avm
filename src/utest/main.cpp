/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#define CATCH_CONFIG_RUNNER
#include <catch.hpp>

int runAllTests(int argc, const char* argv[])
{
	return Catch::Session().run(argc, argv);
}

int main(int argc, const char* argv[])
{
	return runAllTests(argc, argv);
}