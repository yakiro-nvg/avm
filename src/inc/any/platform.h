/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#ifdef _MSC_VER
#define AMSVC _MSC_VER
#define ASTATIC_ASSERT(c) typedef char _astatic_assertion[(c) ? 1 : -1]
#pragma warning(disable: 4127) // conditional expression is constant
#elif defined(__clang__)
#define ACLANG (((__clang_major__)*100) + \
    (__clang_minor__*10) + \
     __clang_patchlevel__)
#define ASTATIC_ASSERT(c) _Static_assert(c, "failed")
#elif defined(__GNUC__)
#define AGNUC (((__GNUC__)*100) + \
    (__GNUC_MINOR__*10) + \
     __GNUC_PATCHLEVEL__)
#define ASTATIC_ASSERT(c) typedef char _astatic_assertion[(c) ? 1 : -1]
#else
#   error "unknown compiler"
#endif

#if defined(__WIN32__) || defined(_WIN32)
#define AWINDOWS
#elif defined(__APPLE_CC__)
#define AAPPLE
#else
#define ALINUX
#endif

#if defined(__x86_64__) || defined(_M_X64) || defined(__arm64__)
#define A64_BIT
#else
#define A32_BIT
#endif

#ifdef AMSVC
#define APACKED
#define AINLINE __inline
#elif defined(ACLANG) || defined(AGNUC)
#define APACKED __attribute__((packed))
#define AINLINE static inline
#endif

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef ANY_SHARED
#define ANY_API
#else
#ifndef ANY_EXPORT
#define ANY_API	__declspec(dllimport)
#else
#define ANY_API __declspec(dllexport)
#endif
#endif

#define AUNUSED(x) ((void)x)

#define ABIG_ENDIAN 0

#ifndef NULL
#define NULL 0
#endif