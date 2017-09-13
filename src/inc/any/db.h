/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/tool_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ADB_API
#define ADB_API ANY_API
#endif

/// Initialize as a new debug service.
ADB_API aerror_t
adb_init(
    adb_t* self, aalloc_t alloc, void* alloc_ud, ascheduler_t* target,
    const char* address, uint16_t port, aint_t max_connections);

/// Release all allocated memory.
ADB_API void
adb_cleanup(
    adb_t* self);

/// Give the debug server a chance to run.
ADB_API void
adb_run_once(
    adb_t* self);

#ifdef __cplusplus
} // extern "C"
#endif