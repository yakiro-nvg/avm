/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/std_buffer_bits.h>

#include <any/gc.h>
#include <any/loader.h>
#include <any/std_buffer.h>

static void
lset_sint_le(
    aactor_t* a)
{
    aint_t cnt = 0;
    aint_t a_self = any_check_index(a, -1);
    aint_t a_val = any_check_index(a, -2);
    aint_t a_startbit = any_check_index(a, -3);
    aint_t a_len = any_check_index(a, -4);
    uint8_t* b = any_check_buffer(a, a_self);
    aint_t val = any_check_integer(a, a_val);
    aint_t idx = any_check_integer(a, a_startbit);
    aint_t len = any_check_integer(a, a_len);
    aint_t bit = 0;
    for (cnt = 0; cnt < len; cnt++)
    {
        // clear bit
        b[(idx + cnt) / 8] &= ~(1 << (idx % 8 + (cnt%8)));
        // set bit
        if (((val >> cnt) & 0x01) == 0x01)
        {
            b[(idx + cnt) / 8] |= 1 << (cnt%8);
        }
    }
    any_push_nil(a);
}

static void
lset_sint_be(
    aactor_t* a)
{
    aint_t cnt = 0;
    aint_t a_self = any_check_index(a, -1);
    aint_t a_val = any_check_index(a, -2);
    aint_t a_startbit = any_check_index(a, -3);
    aint_t a_len = any_check_index(a, -4);
    uint8_t* b = any_check_buffer(a, a_self);
    aint_t val = any_check_integer(a, a_val);
    aint_t idx = any_check_integer(a, a_startbit);
    aint_t len = any_check_integer(a, a_len);
    for (cnt = 0; cnt < len; cnt++)
    {
        // clear bit
        b[(idx + cnt) / 8] &= ~(1 << (8 - (idx % 8 + (cnt % 8))));
        // set bit
        if (((val >> cnt) & 0x01) == 0x01)
        {
            b[(idx + cnt) / 8] |= 1 << (8 - (cnt % 8));
        }
    }
    any_push_nil(a);
}

static void
lset(
    aactor_t* a)
{
    aint_t a_self = any_check_index(a, -1);
    aint_t a_val = any_check_index(a, -2);
    aint_t a_startbit = any_check_index(a, -3);
    uint8_t* b = any_check_buffer(a, a_self);
    aint_t val = any_check_integer(a, a_val);
    aint_t idx = any_check_integer(a, a_startbit);
    if (val == 0) {
        // clear bit
        b[idx / 8] &= ~(1 << (idx % 8));
    }
    else {
        // set bit
        b[idx / 8] |= 1 << (idx % 8);
    }
    any_push_nil(a);
}

static alib_func_t funcs[] = {
    { "set_sint_le/4",  &lset_sint_le },
    { "set_sint_be/4",  &lset_sint_be },
    { "set/3",          &lset },
    { NULL, NULL }
};

static alib_t mod = { "std-buffer-bits", funcs };

void
astd_lib_add_buffer_bits(
    aloader_t* l)
{
    aloader_add_lib(l, &mod);
}