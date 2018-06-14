// Copyright (c) 2017-2018 Giang "Yakiro" Nguyen. All rights reserved.
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
#define __func__ __FUNCTION__
#endif
#pragma warning(disable: 4127) // conditional expression is constant
#elif defined(__clang__)
#define ACLANG (((__clang_major__)*100) + \
    (__clang_minor__*10) + \
     __clang_patchlevel__)
#ifdef __cplusplus
#define ASTATIC_ASSERT(c) static_assert(c, #c)
#else
#define ASTATIC_ASSERT(c) _Static_assert(c, #c)
#endif
#elif defined(__GNUC__)
#define AGNUC (((__GNUC__)*100) + \
    (__GNUC_MINOR__*10) + \
     __GNUC_PATCHLEVEL__)
#ifdef __cplusplus
#define ASTATIC_ASSERT(c) static_assert(c, #c)
#else
#define ASTATIC_ASSERT(c) _Static_assert(c, #c)
#endif
#else
#   error "unknown compiler"
#endif

#ifndef ASTATIC
#define ASTATIC static
#endif

#ifndef AINLINE
#ifdef AMSVC
#define AINLINE static __inline
#else
#define AINLINE static inline
#endif
#endif

#if defined(AMSVC) && AMSVC <= 1500
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
#   ifndef AVM_SHARED
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

#define AMIN(x, y) (((x) < (y)) ? (x) : (y))
#define AMAX(x, y) (((x) > (y)) ? (x) : (y))

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#ifdef ADEBUG
#define AASSERT(c, m) \
    do { \
        if (!(c)) { \
            fprintf(stderr, "[%s:%d] Assert failed in %s(): %s\n", \
                __FILE__, __LINE__, __func__, m); \
            abort(); \
        } \
    } while(FALSE)
#define AVERIFY(c, m) \
    do { \
        if (!(c)) { \
            fprintf(stderr, "[%s:%d] Verify failed in %s(): %s\n", \
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
#define AVERIFY(c, m) (c)
#if defined(AMSVC)
#define AUNREACHABLE() __assume(0)
#elif defined(AGNUC) || defined (ACLANG)
#define AUNREACHABLE() __builtin_unreachable()
#else
#define AUNREACHABLE()
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
    void*(*malloc)(struct aalloc_s *a, u32 sz, u32 align);
    void(*dealloc)(struct aalloc_s *a, void *p);
} aalloc_t;

#define AMAKE(a, T) ((T*)a->malloc(a, sizeof(T), alignof_##T()))
#define AMAKE_ARRAY(a, T, n) ((T*)a->malloc(a, sizeof(T)*n, alignof_##T()))
#define AFREE(a, p) a->dealloc(a, p)

AINLINE void*
align_forward(
    void* p, u32 align)
{
    const uintptr_t pi = (uintptr_t)p;
    const u32 mod = pi % align;
    return mod == 0 ? (void*)pi : (void*)(pi + align - mod);
}

#define AALIGNAS(T, a) \
    AINLINE u32 alignof_##T() { return a; }

/// Primitives alignments
AALIGNAS(char, 1)
AALIGNAS(s8, 4)
AALIGNAS(s16, 4)
AALIGNAS(s32, 4)
AALIGNAS(s64, 8)
AALIGNAS(u8, 4)
AALIGNAS(u16, 4)
AALIGNAS(u32, 4)
AALIGNAS(u64, 8)
AALIGNAS(f32, 4)
AALIGNAS(f64, 8)
AALIGNAS(abool, 4)

#define AVERSION_MAJOR 0
#define AVERSION_MINOR 2
#define AVERSION_PATCH 0
#define AVERSION_NAME "LITTLE.bang"

#endif // !_AVM_PREREQ_H_