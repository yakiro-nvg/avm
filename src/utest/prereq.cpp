/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include "prereq.h"

#include <any/asm.h>
#include <any/actor.h>
#include <any/scheduler.h>
#include <any/std_string.h>

#include <iostream>

void* myalloc(void*, void* old, aint_t sz)
{
    return realloc(old, (size_t)sz);
}

void on_panic(aactor_t* a, void*)
{
    aint_t ev_idx = any_top(a);
    if (any_type(a, ev_idx).type == AVT_STRING) {
        std::cout << "[" << ascheduler_pid(a->owner, a) << "] panic - " <<
            any_to_string(a, ev_idx) << "\n";
    } else {
        std::cout << "[" << ascheduler_pid(a->owner, a) << "] panic - " <<
            "unknown fatal error" << "\n";
    }
}

void add_module(aasm_t* a, const char* name)
{
    aasm_prototype_t* p = aasm_prototype(a);
    p->symbol = aasm_string_to_ref(a, name);
}
