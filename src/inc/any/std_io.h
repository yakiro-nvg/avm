/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/rt_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Add std-io library.
ANY_API void
astd_lib_add_io(
    aloader_t* l, void(*out)(void*, const char*), void* ud);

#ifdef __cplusplus
} // extern "C"
#endif
