/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

typedef enum {
    AERR_NONE       = 0,
    AERR_FULL       = -1,
    AERR_MALFORMED  = -2,
    AERR_UNRESOLVED = -3,
    AERR_OVERFLOW   = -4,
    AERR_RUNTIME    = -5
} aerror_t;