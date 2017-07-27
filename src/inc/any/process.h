/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/rt_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Lookup for a module level symbol and push it onto the stack.
ANY_API void aprocess_find(aprocess_t* p, const char* module, const char* name);

/// Call a function on top of the stack.
AINLINE void aprocess_call(aprocess_t* p)
{
    p->owner->runner->call(p);
}

/// Call a function in protected mode.
ANY_API void aprocess_protected_call(aprocess_t* p);

/// Push a nil value.
ANY_API void aprocess_push_nil(aprocess_t* p);

/// Suspends the execution flow.
AINLINE void aprocess_yield(aprocess_t* p)
{
    ascheduler_t* owner = (ascheduler_t*)p->owner;
    afiber_switch(&p->fiber, &owner->fiber);
}

/// Execute in protected mode.
ANY_API int32_t aprocess_try(aprocess_t* p, void(*f)(aprocess_t*, void*), void* ud);

/// Throw an error.
ANY_API void aprocess_throw(aprocess_t* p, int32_t ec);

/// Throw an error, with description string pushed onto the stack.
ANY_API void aprocess_error(aprocess_t* p, const char* fmt, ...);

/// Naive dispatcher.
ANY_API void any_naive(adispatcher_t* self);

#ifdef __cplusplus
} // extern "C"
#endif