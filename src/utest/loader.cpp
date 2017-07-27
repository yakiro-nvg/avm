/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#ifdef ANY_TOOL

#include <catch.hpp>

#include <any/asm.h>
#include <any/loader.h>
#include <any/prototype.h>
#include <any/errno.h>

static void* myalloc(void*, void* old, int32_t sz)
{
    return realloc(old, sz);
}

static void push_module_a(aasm_t* a)
{
    aasm_prototype_t* const p = aasm_prototype(a);
    p->symbol = aasm_string_to_ref(a, "mod_a");

    aasm_module_push(a, "f1");
    aasm_add_constant(a, ac_integer(0xAF1));
    aasm_add_import(a, "mod_b", "f2");
    aasm_emit(a, ai_nop());
    aasm_emit(a, ai_imp(0));
    aasm_emit(a, ai_ldk(0));
    aasm_emit(a, ai_ret());
    aasm_pop(a);

    aasm_module_push(a, "f2");
    aasm_add_constant(a, ac_integer(0xAF2));
    aasm_add_import(a, "mod_n", "f1");
    aasm_emit(a, ai_nop());
    aasm_emit(a, ai_ldk(0));
    aasm_emit(a, ai_imp(0));
    aasm_emit(a, ai_ret());
    aasm_pop(a);
}

static void push_module_b(aasm_t* b)
{
    aasm_prototype_t* const p = aasm_prototype(b);
    p->symbol = aasm_string_to_ref(b, "mod_b");

    aasm_module_push(b, "f2");
    aasm_add_constant(b, ac_integer(0xBF2));
    aasm_add_import(b, "mod_n", "f2");
    aasm_emit(b, ai_ldk(0));
    aasm_emit(b, ai_imp(0));
    aasm_emit(b, ai_nop());
    aasm_emit(b, ai_ret());
    aasm_pop(b);

    aasm_module_push(b, "f1");
    aasm_add_constant(b, ac_integer(0xBF1));
    aasm_add_import(b, "mod_a", "f1");
    aasm_emit(b, ai_imp(0));
    aasm_emit(b, ai_nop());
    aasm_emit(b, ai_ldk(0));
    aasm_emit(b, ai_ret());
    aasm_pop(b);
}

TEST_CASE("loader_link")
{
    aasm_t a;
    aasm_init(&a, &myalloc, NULL);
    aasm_load(&a, NULL);
    aasm_t b;
    aasm_init(&b, &myalloc, NULL);
    aasm_load(&b, NULL);

    push_module_a(&a);
    push_module_b(&b);

    aasm_save(&a);
    aasm_save(&b);

    static alib_func_t nfuncs[] = {
        { "f1", (anative_func_t)0xF1 },
        { "f2", (anative_func_t)0xF2 },
        { NULL, NULL }
    };
    static alib_t nmodule = { "mod_n", nfuncs };

    aloader_t l;
    aloader_init(&l, &myalloc, NULL);

    REQUIRE(AERR_NONE ==
        aloader_add_chunk(&l, a.chunk, a.chunk_size, NULL, NULL));
    REQUIRE(AERR_NONE ==
        aloader_add_chunk(&l, b.chunk, b.chunk_size, NULL, NULL));
    aloader_add_lib(&l, &nmodule);

    REQUIRE(AERR_NONE == aloader_link(&l));

    avalue_t af1;
    REQUIRE(AERR_NONE == aloader_find(&l, "mod_a", "f1", &af1));
    REQUIRE(af1.tag.collectable == FALSE);
    REQUIRE(af1.tag.b == ABT_FUNCTION);
    REQUIRE(af1.tag.variant == AVTF_AVM);
    REQUIRE(af1.v.avm_func->header->num_instructions == 4);
    REQUIRE(af1.v.avm_func->instructions[0].b.opcode == AOC_NOP);
    REQUIRE(af1.v.avm_func->instructions[1].b.opcode == AOC_IMP);
    REQUIRE(af1.v.avm_func->instructions[1].imp.idx == 0);
    REQUIRE(af1.v.avm_func->instructions[2].b.opcode == AOC_LDK);
    REQUIRE(af1.v.avm_func->instructions[2].ldk.idx == 0);
    REQUIRE(af1.v.avm_func->instructions[3].b.opcode == AOC_RET);
    REQUIRE(af1.v.avm_func->header->num_constants == 1);
    REQUIRE(af1.v.avm_func->constants[0].tag.b == ABT_NUMBER);
    REQUIRE(af1.v.avm_func->constants[0].tag.variant == AVTN_INTEGER);
    REQUIRE(af1.v.avm_func->constants[0].v.integer == 0xAF1);
    REQUIRE(af1.v.avm_func->header->num_imports == 1);
    REQUIRE(af1.v.avm_func->import_values[0].tag.b == ABT_FUNCTION);
    REQUIRE(af1.v.avm_func->import_values[0].tag.variant == AVTF_AVM);
    aprototype_t* af1i = af1.v.avm_func->import_values[0].v.avm_func;

    avalue_t af2;
    REQUIRE(AERR_NONE == aloader_find(&l, "mod_a", "f2", &af2));
    REQUIRE(af2.v.avm_func->header->num_instructions == 4);
    REQUIRE(af2.v.avm_func->instructions[0].b.opcode == AOC_NOP);
    REQUIRE(af2.v.avm_func->instructions[1].b.opcode == AOC_LDK);
    REQUIRE(af2.v.avm_func->instructions[1].ldk.idx == 0);
    REQUIRE(af2.v.avm_func->instructions[2].b.opcode == AOC_IMP);
    REQUIRE(af2.v.avm_func->instructions[2].imp.idx == 0);
    REQUIRE(af2.v.avm_func->instructions[3].b.opcode == AOC_RET);
    REQUIRE(af2.v.avm_func->header->num_constants == 1);
    REQUIRE(af2.v.avm_func->constants[0].tag.b == ABT_NUMBER);
    REQUIRE(af2.v.avm_func->constants[0].tag.variant == AVTN_INTEGER);
    REQUIRE(af2.v.avm_func->constants[0].v.integer == 0xAF2);
    REQUIRE(af2.v.avm_func->header->num_imports == 1);
    REQUIRE(af2.v.avm_func->import_values[0].tag.b == ABT_FUNCTION);
    REQUIRE(af2.v.avm_func->import_values[0].tag.variant == AVTF_NATIVE);
    REQUIRE(af2.v.avm_func->import_values[0].v.func == (anative_func_t)0xF1);

    avalue_t bf2;
    REQUIRE(AERR_NONE == aloader_find(&l, "mod_b", "f2", &bf2));
    REQUIRE(bf2.v.avm_func->header->num_instructions == 4);
    REQUIRE(bf2.v.avm_func->instructions[0].b.opcode == AOC_LDK);
    REQUIRE(bf2.v.avm_func->instructions[0].ldk.idx == 0);
    REQUIRE(bf2.v.avm_func->instructions[1].b.opcode == AOC_IMP);
    REQUIRE(bf2.v.avm_func->instructions[1].imp.idx == 0);
    REQUIRE(bf2.v.avm_func->instructions[2].b.opcode == AOC_NOP);
    REQUIRE(bf2.v.avm_func->instructions[3].b.opcode == AOC_RET);
    REQUIRE(bf2.v.avm_func->header->num_constants == 1);
    REQUIRE(bf2.v.avm_func->constants[0].tag.b == ABT_NUMBER);
    REQUIRE(bf2.v.avm_func->constants[0].tag.variant == AVTN_INTEGER);
    REQUIRE(bf2.v.avm_func->constants[0].v.integer == 0xBF2);
    REQUIRE(bf2.v.avm_func->header->num_imports == 1);
    REQUIRE(bf2.v.avm_func->import_values[0].tag.b == ABT_FUNCTION);
    REQUIRE(bf2.v.avm_func->import_values[0].tag.variant == AVTF_NATIVE);
    REQUIRE(bf2.v.avm_func->import_values[0].v.func == (anative_func_t)0xF2);

    avalue_t bf1;
    REQUIRE(AERR_NONE == aloader_find(&l, "mod_b", "f1", &bf1));
    REQUIRE(bf1.v.avm_func->header->num_instructions == 4);
    REQUIRE(bf1.v.avm_func->instructions[0].b.opcode == AOC_IMP);
    REQUIRE(bf1.v.avm_func->instructions[0].imp.idx == 0);
    REQUIRE(bf1.v.avm_func->instructions[1].b.opcode == AOC_NOP);
    REQUIRE(bf1.v.avm_func->instructions[2].b.opcode == AOC_LDK);
    REQUIRE(bf1.v.avm_func->instructions[2].ldk.idx == 0);
    REQUIRE(bf1.v.avm_func->instructions[3].b.opcode == AOC_RET);
    REQUIRE(bf1.v.avm_func->header->num_constants == 1);
    REQUIRE(bf1.v.avm_func->constants[0].tag.b == ABT_NUMBER);
    REQUIRE(bf1.v.avm_func->constants[0].tag.variant == AVTN_INTEGER);
    REQUIRE(bf1.v.avm_func->constants[0].v.integer == 0xBF1);
    REQUIRE(bf1.v.avm_func->header->num_imports == 1);
    REQUIRE(bf1.v.avm_func->import_values[0].tag.b == ABT_FUNCTION);
    REQUIRE(bf1.v.avm_func->import_values[0].tag.variant == AVTF_AVM);
    aprototype_t* bf1i = bf1.v.avm_func->import_values[0].v.avm_func;

    REQUIRE(af1i == bf2.v.avm_func);
    REQUIRE(bf1i == af1.v.avm_func);

    aloader_cleanup(&l);

    aasm_cleanup(&a);
    aasm_cleanup(&b);
}

TEST_CASE("loader_link_unresolved")
{
    aasm_t a;
    aasm_init(&a, &myalloc, NULL);
    aasm_load(&a, NULL);
    aasm_t b;
    aasm_init(&b, &myalloc, NULL);
    aasm_load(&b, NULL);

    push_module_a(&a);
    push_module_b(&b);

    aasm_save(&a);
    aasm_save(&b);

    aloader_t l;
    aloader_init(&l, &myalloc, NULL);

    SECTION("missing_a")
    {
        static alib_func_t nfuncs[] = {
            { "f1", (anative_func_t)0xF1 },
            { "f2", (anative_func_t)0xF2 },
            { NULL, NULL }
        };
        static alib_t nmodule = { "mod_n", nfuncs };
        aloader_add_lib(&l, &nmodule);
        aloader_add_chunk(&l, b.chunk, b.chunk_size, NULL, NULL);
        REQUIRE(AERR_UNRESOLVED == aloader_link(&l));
    }

    SECTION("missing_b")
    {
        static alib_func_t nfuncs[] = {
            { "f1", (anative_func_t)0xF1 },
            { "f2", (anative_func_t)0xF2 },
            { NULL, NULL }
        };
        static alib_t nmodule = { "mod_n", nfuncs };
        aloader_add_lib(&l, &nmodule);
        aloader_add_chunk(&l, a.chunk, a.chunk_size, NULL, NULL);
        REQUIRE(AERR_UNRESOLVED == aloader_link(&l));
    }

    SECTION("missing_native_0")
    {
        aloader_add_chunk(&l, a.chunk, a.chunk_size, NULL, NULL);
        aloader_add_chunk(&l, b.chunk, b.chunk_size, NULL, NULL);
        REQUIRE(AERR_UNRESOLVED == aloader_link(&l));
    }

    SECTION("missing_native_1")
    {
        achunk_header_t* chunks[] = { a.chunk, b.chunk, NULL };
        int32_t sizes[] = { a.chunk_size, b.chunk_size, 0 };
        static alib_func_t nfuncs[] = {
            { NULL, NULL }
        };
        alib_t nmodule = { "mod_n", nfuncs };
        aloader_add_lib(&l, &nmodule);
        aloader_add_chunk(&l, a.chunk, a.chunk_size, NULL, NULL);
        aloader_add_chunk(&l, b.chunk, b.chunk_size, NULL, NULL);
        REQUIRE(AERR_UNRESOLVED == aloader_link(&l));
    }

    SECTION("missing_native_2")
    {
        static alib_func_t nfuncs[] = {
            { "f1", (anative_func_t)0xF1 },
            { NULL, NULL }
        };
        alib_t nmodule = { "mod_n", nfuncs };
        aloader_add_lib(&l, &nmodule);
        aloader_add_chunk(&l, a.chunk, a.chunk_size, NULL, NULL);
        aloader_add_chunk(&l, b.chunk, b.chunk_size, NULL, NULL);
        REQUIRE(AERR_UNRESOLVED == aloader_link(&l));
    }

    aloader_cleanup(&l);

    aasm_cleanup(&a);
    aasm_cleanup(&b);
}

TEST_CASE("loader_link_rollback")
{
    // TODO
}

#endif // ANT_TOOL