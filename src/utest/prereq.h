/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/platform.h>
#include <catch.hpp>

#include <any/rt_types.h>

enum { CSTACK_SZ = 1024 * 64 };

void* myalloc(void*, void* old, aint_t sz);
void on_panic(aactor_t* a);