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

#ifdef __cplusplus
} // extern "C"
#endif
