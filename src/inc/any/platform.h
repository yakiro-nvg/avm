/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/config.h>

#ifdef _MSC_VER
#define AMSVC _MSC_VER
#define ASTATIC_ASSERT(c) typedef char _astatic_assertion[(c) ? 1 : -1]
#define ASTDCALL __stdcall
#pragma warning(disable: 4127) // conditional expression is constant
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#elif defined(__clang__)
#define ACLANG (((__clang_major__)*100) + \
    (__clang_minor__*10) + \
     __clang_patchlevel__)
#define ASTATIC_ASSERT(c) _Static_assert(c, "failed")
#define ASTDCALL
#elif defined(__GNUC__)
#define AGNUC (((__GNUC__)*100) + \
    (__GNUC_MINOR__*10) + \
     __GNUC_PATCHLEVEL__)
#define ASTATIC_ASSERT(c) typedef char _astatic_assertion[(c) ? 1 : -1]
#define ASTDCALL
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
#define A64_BITS
ASTATIC_ASSERT(sizeof(void*) == 8);
#else
#define A32_BITS
ASTATIC_ASSERT(sizeof(void*) == 4);
#endif

#if defined(_M_IX86) || defined(__i386)
#define AARCH_I386
#elif defined(__x86_64__) || defined(_M_X64)
#define AARCH_AMD64
#elif defined(__arm__)
#define AARCH_ARM
#else
#error "unknown cpu architecture"
#endif

#ifdef AMSVC
#define APACKED
#define AINLINE __inline
#elif defined(ACLANG) || defined(AGNUC)
#define APACKED __attribute__((packed))
#define AINLINE inline
#endif

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef ANY_SHARED
#define ANY_API
#elif defined(AMSVC)
#    ifndef ANY_EXPORT
#        define ANY_API __declspec(dllimport)
#    else
#        define ANY_API __declspec(dllexport)
#    endif
#else
#define ANY_API
#endif

#define AUNUSED(x) ((void)x)

#define ABIG_ENDIAN 0

#ifndef NULL
#define NULL 0
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#ifdef AWINDOWS
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <WinSock2.h>
#endif

#if defined(AMSVC) && !defined(snprintf)
#define snprintf sprintf_s
#endif

#define ACAST_FROM_FIELD(T, n, field) ((T*)(((uint8_t*)n) - offsetof(T, field)))

// Primitive types.
typedef int64_t aint_t;
typedef double areal_t;

#define AALIGN_FORWARD(v, a) ((v + (a - 1)) & -a)
#define ASTATIC_ARRAY_COUNT(arr) (sizeof(arr)/sizeof(arr[0]))