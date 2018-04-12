/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#ifndef _AVM_ERRNO_H_
#define _AVM_ERRNO_H_

typedef enum {
    AR_SUCCESS    = 0,
    AR_MEMORY     = -1,
    AR_MALFORMED  = -2,
    AR_UNRESOLVED = -3,
    AR_RUNTIME    = -4
} aresult_t;

#define ASUCCESS(r) (r >= 0)
#define AFAILED(r)  (r < 0)

#endif // !_AVM_ERRNO_H_