// Copyright (c) 2017-2018 Nguyen Viet Giang. All rights reserved.
#ifndef _AVM_UTILS_H_
#define _AVM_UTILS_H_

#include <avm/prereq.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Compute the next highest power of 2 of 32-bit `v`.
/// http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2Float
AINLINE u32
apowof2_ceil(
    u32 v)
{
    AASSERT(v > 0 && v < 0x80000000, "bad value");
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !_AVM_UTILS_H_