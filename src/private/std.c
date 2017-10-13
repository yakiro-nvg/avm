/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/std.h>

#include <any/actor.h>
#include <any/loader.h>
#include <any/std_string.h>

static AINLINE int32_t
is_number(
    atype_t type)
{
    return type == AVT_INTEGER || type == AVT_REAL;
}

static void
limport(
    aactor_t* a)
{
    aint_t a_module = any_check_index(a, -1);
    aint_t a_name = any_check_index(a, -2);
    const char* module = any_check_string(a, a_module);
    const char* name = any_check_string(a, a_name);
    any_import(a, module, name);
}

static void
lmsleep(
    aactor_t* a)
{
    aint_t a_msecs = any_check_index(a, -1);
    any_sleep(a, any_check_integer(a, a_msecs) * 1000000);
    any_push_nil(a);
}

static void
lusleep(
    aactor_t* a)
{
    aint_t a_usecs = any_check_index(a, -1);
    any_sleep(a, any_check_integer(a, a_usecs) * 1000);
    any_push_nil(a);
}

static void
lnsleep(
    aactor_t* a)
{
    aint_t a_nsecs = any_check_index(a, -1);
    any_sleep(a, any_check_integer(a, a_nsecs));
    any_push_nil(a);
}

static void
lspawn(
    aactor_t* a)
{
    apid_t pid;
    aint_t a_entry = any_check_index(a, -1);
    any_push_index(a, a_entry);
    any_spawn(a, 1024*32, 0, &pid);
    any_push_pid(a, pid);
}

static AINLINE void
is_type(
    aactor_t* a, atype_t type)
{
    aint_t a_val = any_check_index(a, -1);
    avalue_t* v = a->stack.v + a_val;
    any_push_bool(a, v->tag.type == type);
}

static void
lis_integer(
    aactor_t* a)
{
    is_type(a, AVT_INTEGER);
}

static void
lis_real(
    aactor_t* a)
{
    is_type(a, AVT_REAL);
}

static void
lis_boolean(
    aactor_t* a)
{
    is_type(a, AVT_BOOLEAN);
}

static void
lis_buffer(
    aactor_t* a)
{
    is_type(a, AVT_BUFFER);
}
static void
lis_string(
    aactor_t* a)
{
    is_type(a, AVT_STRING);
}
static void
lis_tuple(
    aactor_t* a)
{
    is_type(a, AVT_TUPLE);
}
static void
lis_array(
    aactor_t* a)
{
    is_type(a, AVT_ARRAY);
}

static void
lis_table(
    aactor_t* a)
{
    is_type(a, AVT_TABLE);
}

static void
lis_function(
    aactor_t* a)
{
    aint_t a_val = any_check_index(a, -1);
    avalue_t* v = a->stack.v + a_val;
    any_push_bool(a,
        v->tag.type == AVT_NATIVE_FUNC || v->tag.type == AVT_BYTE_CODE_FUNC);
}

static alib_func_t funcs[] = {
    { "import/2",       &limport },
    { "msleep/1",       &lmsleep },
    { "usleep/1",       &lusleep },
    { "nsleep/1",       &lnsleep },
    { "spawn/1",        &lspawn },
    { "is_integer/1",   &lis_integer },
    { "is_real/1",      &lis_real },
    { "is_boolean/1",   &lis_boolean },
    { "is_buffer/1",    &lis_buffer },
    { "is_string/1",    &lis_string },
    { "is_tuple/1",     &lis_tuple },
    { "is_array/1",     &lis_array },
    { "is_table/1",     &lis_table },
    { "is_function/1",  &lis_function },
    { NULL, NULL }
};

static alib_t mod = { "std", funcs };

void
astd_lib_add(
    aloader_t* l)
{
    aloader_add_lib(l, &mod);
}

int32_t
any_equals(
    aactor_t* a, aint_t lhs_idx, aint_t rhs_idx)
{
    avalue_t* lhs = aactor_at(a, lhs_idx);
    avalue_t* rhs = aactor_at(a, rhs_idx);
    if (lhs->tag.type != rhs->tag.type) {
        if (is_number(lhs->tag.type) && is_number(rhs->tag.type)) {
            return afuzzy_equals(
                any_to_real(a, lhs_idx),
                any_to_real(a, rhs_idx));
        } else {
            return FALSE;
        }
    } else {
        switch (lhs->tag.type) {
        case AVT_BOOLEAN:
            return lhs->v.boolean == rhs->v.boolean;
        case AVT_INTEGER:
            return lhs->v.integer == rhs->v.integer;
        case AVT_REAL:
            return afuzzy_equals(lhs->v.real, rhs->v.real);
        case AVT_STRING:
            return agc_string_compare(a, lhs, rhs) == 0;
        default:
            return FALSE;
        }
    }
}