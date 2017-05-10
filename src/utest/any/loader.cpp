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
    aasm_prototype_t* const p = any_asm_prototype(a);
    p->symbol = any_asm_string_to_ref(a, "mod_a");

    any_asm_module_push(a, "f1");
    any_asm_add_constant(a, ac_integer(0xAF1));
    any_asm_add_import(a, "mod_b", "f2");
    any_asm_emit(a, ai_nop());
    any_asm_emit(a, ai_imp(0));
    any_asm_emit(a, ai_ldk(0));
    any_asm_emit(a, ai_ret());
    any_asm_pop(a);

    any_asm_module_push(a, "f2");
    any_asm_add_constant(a, ac_integer(0xAF2));
    any_asm_add_import(a, "mod_n", "f1");
    any_asm_emit(a, ai_nop());
    any_asm_emit(a, ai_ldk(0));
    any_asm_emit(a, ai_imp(0));
    any_asm_emit(a, ai_ret());
    any_asm_pop(a);
}

static void push_module_b(aasm_t* b)
{
    aasm_prototype_t* const p = any_asm_prototype(b);
    p->symbol = any_asm_string_to_ref(b, "mod_b");

    any_asm_module_push(b, "f2");
    any_asm_add_constant(b, ac_integer(0xBF2));
    any_asm_add_import(b, "mod_n", "f2");
    any_asm_emit(b, ai_ldk(0));
    any_asm_emit(b, ai_imp(0));
    any_asm_emit(b, ai_nop());
    any_asm_emit(b, ai_ret());
    any_asm_pop(b);

    any_asm_module_push(b, "f1");
    any_asm_add_constant(b, ac_integer(0xBF1));
    any_asm_add_import(b, "mod_a", "f1");
    any_asm_emit(b, ai_imp(0));
    any_asm_emit(b, ai_nop());
    any_asm_emit(b, ai_ldk(0));
    any_asm_emit(b, ai_ret());
    any_asm_pop(b);
}

TEST_CASE("loader_link")
{
    aasm_t a;
    any_asm_init(&a, &myalloc, NULL);
    any_asm_load(&a, NULL);
    aasm_t b;
    any_asm_init(&b, &myalloc, NULL);
    any_asm_load(&b, NULL);

    push_module_a(&a);
    push_module_b(&b);

    any_asm_save(&a);
    any_asm_save(&b);

    achunk_t* chunks[] = { a.chunk, b.chunk, NULL };
    int32_t sizes[] = { a.chunk_size, b.chunk_size, 0 };
    static anative_module_func_t nfuncs[] = {
        { "f1", (anative_func_t)0xF1 },
        { "f2", (anative_func_t)0xF2 },
        { NULL, NULL }
    };
    static anative_module_t nmodules[] = {
        { "mod_n", nfuncs },
        { NULL, NULL }
    };
    REQUIRE(AERR_NONE == any_link(chunks, sizes, nmodules));

    const aprototype_t* const mod_a = (const aprototype_t*)(a.chunk + 1);
    REQUIRE(mod_a->num_nesteds == 2);

    const aprototype_t* const af1 = mod_a->resolved.nesteds[0];
    REQUIRE(af1->num_instructions == 4);
    REQUIRE(any_pt_inst(af1, 0).b.opcode == AOC_NOP);
    REQUIRE(any_pt_inst(af1, 1).b.opcode == AOC_IMP);
    REQUIRE(any_pt_inst(af1, 1).imp.idx == 0);
    REQUIRE(any_pt_inst(af1, 2).b.opcode == AOC_LDK);
    REQUIRE(any_pt_inst(af1, 2).ldk.idx == 0);
    REQUIRE(any_pt_inst(af1, 3).b.opcode == AOC_RET);
    REQUIRE(af1->num_constants == 1);
    REQUIRE(any_pt_const(af1, 0)->tag.tag == ATB_NUMBER);
    REQUIRE(any_pt_const(af1, 0)->tag.variant == AVTN_INTEGER);
    REQUIRE(any_pt_const(af1, 0)->v.i == 0xAF1);
    REQUIRE(af1->num_imports == 1);
    REQUIRE(any_pt_import(af1, 0)->tag.tag == ATB_FUNCTION);
    REQUIRE(any_pt_import(af1, 0)->tag.variant == AVTF_PURE);
    const aprototype_t* const af1i = any_pt_import(af1, 0)->v.mf;

    const aprototype_t* const af2 = mod_a->resolved.nesteds[1];
    REQUIRE(af2->num_instructions == 4);
    REQUIRE(any_pt_inst(af2, 0).b.opcode == AOC_NOP);
    REQUIRE(any_pt_inst(af2, 1).b.opcode == AOC_LDK);
    REQUIRE(any_pt_inst(af2, 1).ldk.idx == 0);
    REQUIRE(any_pt_inst(af2, 2).b.opcode == AOC_IMP);
    REQUIRE(any_pt_inst(af2, 2).imp.idx == 0);
    REQUIRE(any_pt_inst(af2, 3).b.opcode == AOC_RET);
    REQUIRE(af2->num_constants == 1);
    REQUIRE(any_pt_const(af2, 0)->tag.tag == ATB_NUMBER);
    REQUIRE(any_pt_const(af2, 0)->tag.variant == AVTN_INTEGER);
    REQUIRE(any_pt_const(af2, 0)->v.i == 0xAF2);
    REQUIRE(af2->num_imports == 1);
    REQUIRE(any_pt_import(af2, 0)->tag.tag == ATB_FUNCTION);
    REQUIRE(any_pt_import(af2, 0)->tag.variant == AVTF_NATIVE);
    REQUIRE(any_pt_import(af2, 0)->v.f == (anative_func_t)0xF1);

    const aprototype_t* const mod_b = (const aprototype_t*)(b.chunk + 1);
    REQUIRE(mod_b->num_nesteds == 2);

    const aprototype_t* const bf2 = mod_b->resolved.nesteds[0];
    REQUIRE(bf2->num_instructions == 4);
    REQUIRE(any_pt_inst(bf2, 0).b.opcode == AOC_LDK);
    REQUIRE(any_pt_inst(bf2, 0).ldk.idx == 0);
    REQUIRE(any_pt_inst(bf2, 1).b.opcode == AOC_IMP);
    REQUIRE(any_pt_inst(bf2, 1).imp.idx == 0);
    REQUIRE(any_pt_inst(bf2, 2).b.opcode == AOC_NOP);
    REQUIRE(any_pt_inst(bf2, 3).b.opcode == AOC_RET);
    REQUIRE(bf2->num_constants == 1);
    REQUIRE(any_pt_const(bf2, 0)->tag.tag == ATB_NUMBER);
    REQUIRE(any_pt_const(bf2, 0)->tag.variant == AVTN_INTEGER);
    REQUIRE(any_pt_const(bf2, 0)->v.i == 0xBF2);
    REQUIRE(bf2->num_imports == 1);
    REQUIRE(any_pt_import(bf2, 0)->tag.tag == ATB_FUNCTION);
    REQUIRE(any_pt_import(bf2, 0)->tag.variant == AVTF_NATIVE);
    REQUIRE(any_pt_import(bf2, 0)->v.f == (anative_func_t)0xF2);

    const aprototype_t* const bf1 = mod_b->resolved.nesteds[1];
    REQUIRE(bf1->num_instructions == 4);
    REQUIRE(any_pt_inst(bf1, 0).b.opcode == AOC_IMP);
    REQUIRE(any_pt_inst(bf1, 0).imp.idx == 0);
    REQUIRE(any_pt_inst(bf1, 1).b.opcode == AOC_NOP);
    REQUIRE(any_pt_inst(bf1, 2).b.opcode == AOC_LDK);
    REQUIRE(any_pt_inst(bf1, 2).ldk.idx == 0);
    REQUIRE(any_pt_inst(bf1, 3).b.opcode == AOC_RET);
    REQUIRE(bf1->num_constants == 1);
    REQUIRE(any_pt_const(bf1, 0)->tag.tag == ATB_NUMBER);
    REQUIRE(any_pt_const(bf1, 0)->tag.variant == AVTN_INTEGER);
    REQUIRE(any_pt_const(bf1, 0)->v.i == 0xBF1);
    REQUIRE(bf1->num_imports == 1);
    REQUIRE(any_pt_import(bf1, 0)->tag.tag == ATB_FUNCTION);
    REQUIRE(any_pt_import(bf1, 0)->tag.variant == AVTF_PURE);
    const aprototype_t* const bf1i = any_pt_import(bf1, 0)->v.mf;

    REQUIRE(af1i == bf2);
    REQUIRE(bf1i == af1);

    any_asm_cleanup(&a);
    any_asm_cleanup(&b);
}

TEST_CASE("loader_link_unresolved")
{
    aasm_t a;
    any_asm_init(&a, &myalloc, NULL);
    any_asm_load(&a, NULL);
    aasm_t b;
    any_asm_init(&b, &myalloc, NULL);
    any_asm_load(&b, NULL);

    push_module_a(&a);
    push_module_b(&b);

    any_asm_save(&a);
    any_asm_save(&b);

    SECTION("missing_a")
    {
        achunk_t* chunks[] = { b.chunk, NULL };
        int32_t sizes[] = { b.chunk_size, 0 };
        static anative_module_func_t nfuncs[] = {
            { "f1", (anative_func_t)0xF1 },
            { "f2", (anative_func_t)0xF2 },
            { NULL, NULL }
        };
        static anative_module_t nmodules[] = {
            { "mod_n", nfuncs },
            { NULL, NULL }
        };
        REQUIRE(AERR_UNRESOLVED == any_link(chunks, sizes, nmodules));
    }

    SECTION("missing_b")
    {
        achunk_t* chunks[] = { a.chunk, NULL };
        int32_t sizes[] = { a.chunk_size, 0 };
        static anative_module_func_t nfuncs[] = {
            { "f1", (anative_func_t)0xF1 },
            { "f2", (anative_func_t)0xF2 },
            { NULL, NULL }
        };
        static anative_module_t nmodules[] = {
            { "mod_n", nfuncs },
            { NULL, NULL }
        };
        REQUIRE(AERR_UNRESOLVED == any_link(chunks, sizes, nmodules));
    }

    SECTION("missing_native_0")
    {
        achunk_t* chunks[] = { a.chunk, b.chunk, NULL };
        int32_t sizes[] = { a.chunk_size, b.chunk_size, 0 };
        REQUIRE(AERR_UNRESOLVED == any_link(chunks, sizes, NULL));
    }

    SECTION("missing_native_1")
    {
        achunk_t* chunks[] = { a.chunk, b.chunk, NULL };
        int32_t sizes[] = { a.chunk_size, b.chunk_size, 0 };
        anative_module_t nmodules[] = {
            { NULL, NULL }
        };
        REQUIRE(AERR_UNRESOLVED == any_link(chunks, sizes, nmodules));
    }

    SECTION("missing_native_2")
    {
        achunk_t* chunks[] = { a.chunk, b.chunk, NULL };
        int32_t sizes[] = { a.chunk_size, b.chunk_size, 0 };
        static anative_module_func_t nfuncs[] = {
            { NULL, NULL }
        };
        anative_module_t nmodules[] = {
            { "mod_n", nfuncs },
            { NULL, NULL }
        };
        REQUIRE(AERR_UNRESOLVED == any_link(chunks, sizes, nmodules));
    }

    SECTION("missing_native_3")
    {
        achunk_t* chunks[] = { a.chunk, b.chunk, NULL };
        int32_t sizes[] = { a.chunk_size, b.chunk_size, 0 };
        static anative_module_func_t nfuncs[] = {
            { "f1", (anative_func_t)0xF1 },
            { NULL, NULL }
        };
        anative_module_t nmodules[] = {
            { "mod_n", nfuncs },
            { NULL, NULL }
        };
        REQUIRE(AERR_UNRESOLVED == any_link(chunks, sizes, nmodules));
    }

    any_asm_cleanup(&a);
    any_asm_cleanup(&b);
}

#endif // ANT_TOOL