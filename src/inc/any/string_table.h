/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initializes an empty string table in the specified memory area. `bytes` is
// the total ammount of memory allocated at the pointer and `average_strlen` is
// the expected average length of the strings that will be added.
ANY_API void any_st_init(
    astring_table_t* self, int32_t bytes, int32_t average_strlen);

// Grows the string table to size `bytes`. You must make sure that this many
// bytes are available in the pointer `self`, typically by realloc before
// calling this function.
ANY_API void any_st_grow(astring_table_t* self, int32_t bytes);

// Packs the string table so that it uses as little memory as possible while
// still preserving the content. Updates self->allocated_bytes and returns the
// new value. You can use that to shrink the buffer with realloc if so desired.
ANY_API int32_t any_st_pack(astring_table_t* self);

// Returns the reference for the string `s`. If `s` is not already in the table,
// it is added. If `s` can't be added because the table is full, the function
// returns `AERR_FULL`.
//
// The empty string is guaranteed to have the reference `0`.
ANY_API astring_ref_t any_st_to_ref(astring_table_t* self, const char* s);

// As any_st_to_ref(), but never adds the string to the table.
// If the string doesn't exist in the table AERR_FULL is returned.
ANY_API astring_ref_t any_st_to_ref_const(
    const astring_table_t* self, const char* s);

// Returns the string corresponding to the `ref`. Calling this with a
// value which is not a ref returned by `any_st_to_ref` results in
// undefined behavior.
ANY_API const char* any_st_to_string(astring_table_t* self, astring_ref_t ref);

// Returns the hashed value corresponding the the `ref`. Calling this
// with a value which is not a ref returned by `any_st_to_ref` results
// in undefined behavior.
ANY_API uint32_t any_st_to_hash(astring_table_t* self, astring_ref_t ref);

// Utility function to calculate the hash and length of string `s`.
AINLINE ahash_and_length_t ahash_and_length(const char* s)
{
    // The hash function is borrowed from Lua.
    // Since we need to walk the entire string anyway for finding the
    // length, this is a decent hash function.
    uint32_t h = 0;
    const char* i = s;
    for (; *i; ++i) h = h ^ ((h << 5) + (h >> 2) + (unsigned char)*i);
    ahash_and_length_t result;
    result.hash = h;
    result.length = i - s;
    return result;
}

#ifdef __cplusplus
} // extern "C"
#endif