/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <assert.h>
#include <any/rt_types.h>
#include <any/process.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Utility function to calculate the hash and length of string `s`.
ANY_API ahash_and_length_t ahash_and_length(const char* string);

#ifdef __cplusplus
} // extern "C"
#endif