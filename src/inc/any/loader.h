/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/rt_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Link multiple chunk of bytecode together.
\brief 
This function will validate these chunks and try to resolve all required pointers.
After that, this chunk prototypes becomes ready to execute, with correct resolved
\ref aimport_t and \ref acurrent_t pointers. This function is `pure`, that means 
this can't not touch other variables but only input. Which also means no global
state and no dynamic memory allocation required.

This design make somethings simpler and somethings trickier. If we only have a few
chunks which are embedded inside the application, and be statically allocated at 
the compile time, then that will be really straight forward. However, things will
become harder when we needed to reload/upgrade bytecode in runtime. There are no 
way to reclaim the allocated memory of successfully linked chunk at this level. 
Obviously we need to rely on something at higher level which in turn traversal all 
values in the stack and heap to determine what chunk is unreferenced so that can 
not be imported anymore. Otherwise we will lost this memory chunk forever, which 
may just `reasonable right` or `seriously wrong`. That is case by case and is up 
to you to decide. Generally speaking, just move on.

This design also do not take care of rolling back these chunks to its previously
good working state, since that isn't mandatory at all. There are dumb use cases 
and hardware which aren't ever willing to support hot bytecode reloading. That 
things also are the targets of AVM, so we must understand that a portable device 
which requires fast bring ups time are really different from web or game servers. 
Therefore, you are responsible to backup your chunks first if required.

Another issue is slow looking up time for prototype from name, since we don't ever 
try to build a hash table at all. Therefore, reflection should also be handle by 
another component if you ever wish to invoke a module level function by reflection. 
Which responsible to making a hash table for you. However, that use cases is rare,
usually we only need to lookup once time to find the entry point at the startup. 
Other references are resolved using \ref aimport_t by this function.

\return `AERR_NONE` if successful.
*/
ANY_API int32_t any_link(
    achunk_t** chunks,  int32_t* sizes, const anative_module_t* natives);

#ifdef __cplusplus
} // extern "C"
#endif