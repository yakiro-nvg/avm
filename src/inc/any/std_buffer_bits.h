/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/rt_types.h>
#include <any/actor.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Add std-buffer-bits library.
ANY_API void
astd_lib_add_buffer_bits(
    aloader_t* l);

#ifdef __cplusplus
} // extern "C"
#endif
