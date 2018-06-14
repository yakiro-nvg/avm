// Copyright (c) 2017-2018 Giang "Yakiro" Nguyen. All rights reserved.
#ifndef _AVM_ALLOC_H_
#define _AVM_ALLOC_H_

#include <avm/prereq.h>
#include <avm/errno.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Returns a NULL allocator.
AINLINE aalloc_t
aalloc_null()
{
    aalloc_t a;
    a.malloc = NULL;
    return a;
}

/// Check whether this allocator is NULL.
AINLINE abool
aalloc_is_null(
    aalloc_t *a)
{
    return a->malloc == NULL;
}

/// Standard malloc based allocator.
typedef struct amalloc_s {
    aalloc_t aif;
    u32 allocated;
} amalloc_t;

/// Initialize as a new allocator.
AAPI aresult_t
amalloc_init(
    amalloc_t *a);

/// Gracefully cleanup.
AAPI void
amalloc_release(
    amalloc_t *a);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !_AVM_ALLOC_H_