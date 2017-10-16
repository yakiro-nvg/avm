/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/rt_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Add std library.
ANY_API void
astd_lib_add(
    aloader_t* l);

/// Compare real with tolerance.
static AINLINE int32_t
afuzzy_equals(
    areal_t a, areal_t b)
{
    areal_t precision = (areal_t)0.00001;
    return (a - precision) < b && (a + precision) > b;
}

/// Compare two values.
ANY_API int32_t
any_equals(
    aactor_t* a, aint_t lhs_idx, aint_t rhs_idx);

#ifdef __cplusplus
} // extern "C"
#endif
