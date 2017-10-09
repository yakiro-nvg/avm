/* Copyright (c) 2017 Nguyen Nhan Tinh. All rights reserved. */
#include <any/std_buffer.h>

#include <any/gc.h>
#include <any/loader.h>

#define GROW_FACTOR 2
#define INIT_GROW 64

static void
lset_int_le_bits(
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
	if (idx % 8 + len > 8)
	{
		b[idx/8] = b[idx/8] | ((val << (idx % 8)) & 0x00FF);
		b[idx/8 + 1] = b[idx/8 + 1] | ((val >> (idx%8 + len - 8)) & 0x00FF);
	}
	else
	{
		b[idx / 8] = b[idx / 8] | (val << (idx % 8));
	}
	any_push_nil(a);
}

static void
lset_int_be_bits(
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
	if (idx % 8 + len > 8)
	{
		b[idx / 8] = b[idx / 8] | ((val << (idx % 8)) & 0x00FF);
		b[idx / 8 + 1] = b[idx / 8 + 1] | ((val >> (idx % 8 + len - 8)) & 0x00FF);
	}
	else
	{
		b[idx / 8] = b[idx / 8] | (val << (idx % 8));
	}
	any_push_nil(a);
}

static void
lset_r2i_bits(
	aactor_t* a)
{
	aint_t aint_val;
	areal_t a_val = any_check_real(a, -1);
	areal_t a_min = any_check_real(a, -2);
	areal_t a_rslt = any_check_real(a, -3);
	aint_val = (aint_t)((a_val - a_min) / a_rslt);
	any_push_integer(a, aint_val);
}


static alib_func_t funcs[] = {
	{ "setintlebit/4",      &lset_int_le_bits },
	{ "setintbebit/4",      &lset_int_le_bits },
	{ "convr2i/3",			&lset_r2i_bits },
	{ NULL, NULL }
};

static alib_t mod = { "std-buffer-bits", funcs };

void
astd_lib_add_buffer_bits(
	aloader_t* l)
{
	aloader_add_lib(l, &mod);
}