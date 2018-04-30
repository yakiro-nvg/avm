// Copyright (c) 2017-2018 Nguyen Viet Giang. All rights reserved.
#include "prereq.h"
#include <utils.h>

TEST_CASE("power_of_2_ceil")
{
    REQUIRE(check([](u32 v) {
        RC_PRE(v > 0u && v <= 0x80000000u);
        u32 p = 0;
        for (int i = 0; i < 32; ++i) {
            p = (u32)pow(2, i);
            if (p >= v) break;
        }
        RC_ASSERT(p == apowof2_ceil(v));
    }));
}