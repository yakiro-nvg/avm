/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/rt_types.h>

#ifdef __cplusplus
extern "C" {
#endif

AINLINE const char* any_pt_source(const aprototype_t* p)
{
    return ((const char*)(p + 1)) + p->source;
}

AINLINE const char* any_pt_symbol(const aprototype_t* p)
{
    return ((const char*)(p + 1)) + p->symbol;
}

AINLINE const ainstruction_t any_pt_inst(const aprototype_t* p, int32_t idx)
{
    return p->resolved.instructions[idx];
}

AINLINE const avalue_t* any_pt_const(const aprototype_t* p, int32_t idx)
{
    return p->resolved.constants + idx;
}

AINLINE const avalue_t* any_pt_import(const aprototype_t* p, int32_t idx)
{
    return &p->resolved.imports[idx].resolved;
}

#ifdef __cplusplus
} // extern "C"
#endif