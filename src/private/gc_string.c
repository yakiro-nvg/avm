/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/gc_string.h>

ahash_and_length_t ahash_and_length(const char* s)
{
    // The hash function is borrowed from Lua.
    // Since we need to walk the entire string anyway for finding the
    // length, this is a decent hash function.
    ahash_and_length_t result;
    uint32_t h = 0;
    const char* i = s;
    for (; *i; ++i) h = h ^ ((h << 5) + (h >> 2) + (unsigned char)*i);
    result.hash = h;
    result.length = (int32_t)(i - s);
    return result;
}