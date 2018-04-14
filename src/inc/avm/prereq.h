// Copyright (c) 2017-2018 Nguyen Viet Giang. All rights reserved.
#ifndef _AVM_PREREQ_H_
#define _AVM_PREREQ_H_

#include <avm/config.h>

#ifdef _MSC_VER
#define AMSVC _MSC_VER
#ifndef ASTATIC_ASSERT
#if _MSC_VER <= 1500
#define ASTATIC_ASSERT(c) typedef char _astatic_assertion[(c) ? 1 : -1]
#else
#define ASTATIC_ASSERT(c) static_assert(c, #c)
#endif
#endif
#pragma warning(disable: 4127) // conditional expression is constant
#elif defined(__clang__)
#define ACLANG (((__clang_major__)*100) + \
    (__clang_minor__*10) + \
     __clang_patchlevel__)
#define ASTATIC_ASSERT(c) _Static_assert(c, #c)
#elif defined(__GNUC__)
#define AGNUC (((__GNUC__)*100) + \
    (__GNUC_MINOR__*10) + \
     __GNUC_PATCHLEVEL__)
#define ASTATIC_ASSERT(c) _Static_assert(c, #c)
#else
#   error "unknown compiler"
#endif

#if defined(ATEST) && defined(AMSVC)
#undef AMSVC
#endif

#ifdef AMSVC
#include <avm/msvc_stdint.h>
#else
#include <stdint.h>
#endif
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;
typedef float    f32;
typedef double   f64;
typedef int32_t  abool;

#ifdef  ATEST
#undef  ASTATIC_ASSERT
#define ASTATIC_ASSERT(x)
#endif

#if defined(__WIN32__) || defined(_WIN32)
#define AWINDOWS
#elif defined(__APPLE_CC__)
#define AAPPLE
#else
#define ALINUX
#endif

#if defined(__x86_64__) || defined(_M_X64) || defined(__arm64__)
#define A64BIT
ASTATIC_ASSERT(sizeof(void*) == 8);
#else
#define A32BIT
ASTATIC_ASSERT(sizeof(void*) == 4);
#endif

#ifndef AAPI
#   ifndef ASHARED
#       define AAPI
#   elif defined(AMSVC)
#       ifndef AEXPORT
#           define AAPI __declspec(dllimport)
#       else
#           define AAPI __declspec(dllexport)
#       endif
#   else
#       define AAPI
#   endif
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef AINLINE
#ifdef AMSVC
#define AINLINE static __inline
#else
#define AINLINE static inline
#endif
#endif

#ifdef ADEBUG
#include <stdio.h>
#define AASSERT(c, m) \
    do { \
        if (!(c)) { \
            fprintf(stderr, "[%s:%d] Assert failed in %s(): %s\n", \
                __FILE__, __LINE__, __func__, m); \
            abort(); \
        } \
    } while(FALSE)
#define AUNREACHABLE() \
    do { \
        fprintf(stderr, "[%s:%d] This code should not be reached in %s(): %s\n", \
                __FILE__, __LINE__, __func__); \
        abort(); \
    } while(FALSE)
#else
#define AASSERT(c, m) do {} while (FALSE)
#if defined(AMSVC)
#define AUNREACHABLE() __assume(0)
#elif defined(AGNUC) || defined (ACLANG)
#define AUNREACHABLE() __builtin_unreachable()
#else
#deifne AUNREACHABLE()
#endif
#endif

#define AUNUSED(x) ((void)x)
#define AFROM_FIELD(T, n, field) ((T*)(((uint8_t*)n) - offsetof(T, field)))

/** Allocator interface.
\brief
`old` = 0 to malloc,
`sz`  = 0 to free,
otherwise realloc.
*/
typedef struct aalloc_s {
    void*(*realloc)(struct aalloc_s *a, void *old, u32 sz);
} aalloc_t;
#define AMAKE(a, T)         ((T*)(a->realloc(a, NULL, sizeof(T))))
#define AFREE(a, p)         a->realloc(a, p, 0)
#define AREALLOC(a, p, sz)  a->realloc(a, p, sz)

#define AVERSION_MAJOR 0
#define AVERSION_MINOR 2
#define AVERSION_NAME "LITTLE.bang"

#endif // !_AVM_PREREQ_H_