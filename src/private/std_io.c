/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/std_libs.h>

#include <any/actor.h>
#include <any/loader.h>
#include <any/gc_string.h>

static void(*out)(void*, const char*) = NULL;
static void* out_ud = NULL;

static void remove_trailing_zeroes(char* buf)
{
    char* dot = strchr(buf, '.');
    if (dot != NULL) {
        char* c = dot + strlen(dot) - 1;
        for (; *c == '0'; --c) {
            *c = '\0';
        }
    }
}

static void print(aactor_t* a)
{
    aint_t i;
    aint_t nargs = any_nargs(a);
    char buf[1024]; // TODO: dynamic sized?
    any_push_nil(a);
    if (out == NULL) return;
    for (i = 1; i <= nargs; ++i) {
        switch (any_type(a, -i).type) {
        case AVT_NIL:
            out(out_ud, "nil");
            break;
        case AVT_PID:
            snprintf(buf, sizeof(buf), "<0x%x>", any_to_pid(a, -i));
            out(out_ud, buf);
            break;
        case AVT_BOOLEAN:
            snprintf(buf, sizeof(buf), "%s",
                any_to_bool(a, -i) ? "true" : "false");
            out(out_ud, buf);
            break;
        case AVT_POINTER:
            out(out_ud, "<pointer>");
            break;
        case AVT_INTEGER:
            snprintf(buf, sizeof(buf), "%lld", any_to_integer(a, -i));
            out(out_ud, buf);
            break;
        case AVT_REAL:
            snprintf(buf, sizeof(buf), "%f", any_to_real(a, -i));
            remove_trailing_zeroes(buf);
            out(out_ud, buf);
            break;
        case AVT_NATIVE_FUNC:
            out(out_ud, "<native function>");
            break;
        case AVT_BYTE_CODE_FUNC:
            out(out_ud, "<function>");
            break;
        case AVT_FIXED_BUFFER:
            out(out_ud, "<fixed buffer>");
            break;
        case AVT_BUFFER:
            out(out_ud, "<buffer>");
            break;
        case AVT_STRING:
            snprintf(buf, sizeof(buf), "%s", any_to_string(a, -i));
            out(out_ud, buf);
            break;
        case AVT_TUPLE:
            out(out_ud, "<tuple>");
            break;
        case AVT_ARRAY:
            out(out_ud, "<array>");
            break;
        case AVT_TABLE:
            out(out_ud, "<table>");
            break;
        }
    }
}

static alib_func_t funcs[] = {
    { "print/*", &print },
    { NULL, NULL }
};

static alib_t mod = { "io", funcs };

void astd_lib_add_io(aloader_t* l, void(*o)(void*, const char*), void* ud)
{
    aloader_add_lib(l, &mod);
    out = o;
    out_ud = ud;
}