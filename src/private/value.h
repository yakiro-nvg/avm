// Copyright (c) 2017-2018 Giang "Yakiro" Nguyen. All rights reserved.
#ifndef _AVM_VALUE_H_
#define _AVM_VALUE_H_

#include <avm/prereq.h>

#ifdef __cplusplus
extern "C" {
#endif

/// NaN boxed value.
typedef f64 avalue_t;
AALIGNAS(avalue_t, 8)

/// Value size is fixed 8 bytes.
ASTATIC_ASSERT(sizeof(avalue_t) == 8);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !_AVM_VALUE_H_