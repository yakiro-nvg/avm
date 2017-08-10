/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/rt_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Initialize as a new dispatcher.
ANY_API aerror_t adispatcher_init(adispatcher_t* self, aprocess_t* owner);

/// Execute current frame.
ANY_API void adispatcher_call(adispatcher_t* self);

#ifdef __cplusplus
} // extern "C"
#endif