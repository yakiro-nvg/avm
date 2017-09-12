/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/platform.h>
#include <catch.hpp>

#include <any/types.h>

enum { CSTACK_SZ = 1024 * 512 };

void* myalloc(void*, void* old, aint_t sz);
void on_panic(aactor_t* a, void* ud);
void add_module(aasm_t* a, const char* name);