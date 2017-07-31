/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#ifdef ANY_TOOL

#include <catch.hpp>

#include <any/errno.h>
#include <any/asm.h>
#include <any/loader.h>
#include <any/prototype.h>
#include <any/list.h>

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

static void push_module_c(aasm_t* c)
{
    aasm_prototype_t* const p = aasm_prototype(c);
    p->symbol = aasm_string_to_ref(c, "mod_c");

    aasm_module_push(c, "f1");
    aasm_add_constant(c, ac_integer(0xCF1));
    aasm_add_import(c, "mod_a", "f3");
    aasm_emit(c, ai_nop());
    aasm_emit(c, ai_ret());
    aasm_pop(c);
}

static bool compare_vtag(avalue_t* a, avalue_t* b)
{
    return a->tag.b == b->tag.b && a->tag.variant == b->tag.variant;
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

    REQUIRE(AERR_NONE == aloader_link(&l, FALSE));

    avalue_t af1;
    REQUIRE(AERR_NONE == aloader_find(&l, "mod_a", "f1", &af1));
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
    REQUIRE(af1.v.avm_func->constants[0].b.type == ACT_INTEGER);
    REQUIRE(af1.v.avm_func->constants[0].integer.val == 0xAF1);
    REQUIRE(af1.v.avm_func->header->num_imports == 1);
    REQUIRE(af1.v.avm_func->import_values[0].tag.b == ABT_FUNCTION);
    REQUIRE(af1.v.avm_func->import_values[0].tag.variant == AVTF_AVM);
    avalue_t af1i = af1.v.avm_func->import_values[0];

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
    REQUIRE(af2.v.avm_func->constants[0].b.type == ACT_INTEGER);
    REQUIRE(af2.v.avm_func->constants[0].integer.val == 0xAF2);
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
    REQUIRE(bf2.v.avm_func->constants[0].b.type == ACT_INTEGER);
    REQUIRE(bf2.v.avm_func->constants[0].integer.val == 0xBF2);
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
    REQUIRE(bf1.v.avm_func->constants[0].b.type == ACT_INTEGER);
    REQUIRE(bf1.v.avm_func->constants[0].integer.val == 0xBF1);
    REQUIRE(bf1.v.avm_func->header->num_imports == 1);
    REQUIRE(bf1.v.avm_func->import_values[0].tag.b == ABT_FUNCTION);
    REQUIRE(bf1.v.avm_func->import_values[0].tag.variant == AVTF_AVM);
    avalue_t bf1i = bf1.v.avm_func->import_values[0];

    REQUIRE(compare_vtag(&af1i, &bf2));
    REQUIRE(af1i.v.avm_func == bf2.v.avm_func);
    REQUIRE(compare_vtag(&bf1i, &af1));
    REQUIRE(bf1i.v.avm_func == af1.v.avm_func);

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
        REQUIRE(AERR_UNRESOLVED == aloader_link(&l, FALSE));
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
        REQUIRE(AERR_UNRESOLVED == aloader_link(&l, FALSE));
    }

    SECTION("missing_native_0")
    {
        aloader_add_chunk(&l, a.chunk, a.chunk_size, NULL, NULL);
        aloader_add_chunk(&l, b.chunk, b.chunk_size, NULL, NULL);
        REQUIRE(AERR_UNRESOLVED == aloader_link(&l, FALSE));
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
        REQUIRE(AERR_UNRESOLVED == aloader_link(&l, FALSE));
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
        REQUIRE(AERR_UNRESOLVED == aloader_link(&l, FALSE));
    }

    aloader_cleanup(&l);

    aasm_cleanup(&a);
    aasm_cleanup(&b);
}

TEST_CASE("loader_link_safe_and_sweep")
{
    aasm_t a;
    aasm_init(&a, &myalloc, NULL);
    aasm_load(&a, NULL);
    aasm_t b;
    aasm_init(&b, &myalloc, NULL);
    aasm_load(&b, NULL);
    aasm_t c;
    aasm_init(&c, &myalloc, NULL);
    aasm_load(&c, NULL);

    push_module_a(&a);
    push_module_b(&b);
    push_module_c(&c);

    aasm_save(&a);
    aasm_save(&b);
    aasm_save(&c);

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

    REQUIRE(AERR_NONE == aloader_link(&l, FALSE));

    avalue_t oaf1, oaf2, obf1, obf2, onf1, onf2;
    REQUIRE(AERR_NONE == aloader_find(&l, "mod_a", "f1", &oaf1));
    REQUIRE(AERR_NONE == aloader_find(&l, "mod_a", "f2", &oaf2));
    REQUIRE(AERR_NONE == aloader_find(&l, "mod_b", "f1", &obf1));
    REQUIRE(AERR_NONE == aloader_find(&l, "mod_b", "f2", &obf2));
    REQUIRE(AERR_NONE == aloader_find(&l, "mod_n", "f1", &onf1));
    REQUIRE(AERR_NONE == aloader_find(&l, "mod_n", "f2", &onf2));

    REQUIRE(alist_is_end(&l.pendings, alist_head(&l.pendings)));
    REQUIRE(AERR_NONE ==
        aloader_add_chunk(&l, c.chunk, c.chunk_size, NULL, NULL));
    // link without mod_a.f3
    REQUIRE(AERR_UNRESOLVED == aloader_link(&l, TRUE));
    REQUIRE(alist_is_end(&l.pendings, alist_head(&l.pendings)));

    avalue_t cf1;
    REQUIRE(AERR_UNRESOLVED == aloader_find(&l, "mod_c", "f1", &cf1));

    avalue_t naf1, naf2, nbf1, nbf2, nnf1, nnf2;
    REQUIRE(AERR_NONE == aloader_find(&l, "mod_a", "f1", &naf1));
    REQUIRE(AERR_NONE == aloader_find(&l, "mod_a", "f2", &naf2));
    REQUIRE(AERR_NONE == aloader_find(&l, "mod_b", "f1", &nbf1));
    REQUIRE(AERR_NONE == aloader_find(&l, "mod_b", "f2", &nbf2));
    REQUIRE(AERR_NONE == aloader_find(&l, "mod_n", "f1", &nnf1));
    REQUIRE(AERR_NONE == aloader_find(&l, "mod_n", "f2", &nnf2));

    REQUIRE(compare_vtag(&oaf1, &naf1));
    REQUIRE(oaf1.v.avm_func == naf1.v.avm_func);
    REQUIRE(compare_vtag(&oaf2, &naf2));
    REQUIRE(oaf2.v.avm_func == naf2.v.avm_func);
    REQUIRE(compare_vtag(&obf1, &nbf1));
    REQUIRE(obf1.v.avm_func == nbf1.v.avm_func);
    REQUIRE(compare_vtag(&obf2, &nbf2));
    REQUIRE(obf2.v.avm_func == nbf2.v.avm_func);
    REQUIRE(compare_vtag(&onf1, &nnf1));
    REQUIRE(onf1.v.func == nnf1.v.func);
    REQUIRE(compare_vtag(&onf2, &nnf2));
    REQUIRE(onf2.v.func == nnf2.v.func);

    // link with mod_a.f3 (reload)
    aasm_t aa;
    aasm_init(&aa, &myalloc, NULL);
    aasm_load(&aa, NULL);
    push_module_a(&aa);
    aasm_module_push(&aa, "f3");
    aasm_add_constant(&aa, ac_integer(0xAF3));
    aasm_add_import(&aa, "mod_b", "f2");
    aasm_emit(&aa, ai_nop());
    aasm_emit(&aa, ai_ldk(0));
    aasm_emit(&aa, ai_imp(0));
    aasm_emit(&aa, ai_ret());
    aasm_pop(&aa);
    aasm_save(&aa);
    REQUIRE(AERR_NONE ==
        aloader_add_chunk(&l, aa.chunk, aa.chunk_size, NULL, NULL));
    REQUIRE(AERR_NONE ==
        aloader_add_chunk(&l, c.chunk, c.chunk_size, NULL, NULL));
    REQUIRE(AERR_NONE == aloader_link(&l, TRUE));

    REQUIRE(AERR_NONE == aloader_find(&l, "mod_c", "f1", &cf1));
    REQUIRE(cf1.v.avm_func->header->num_instructions == 2);
    REQUIRE(cf1.v.avm_func->instructions[0].b.opcode == AOC_NOP);
    REQUIRE(cf1.v.avm_func->instructions[1].b.opcode == AOC_RET);
    REQUIRE(cf1.v.avm_func->header->num_constants == 1);
    REQUIRE(cf1.v.avm_func->constants[0].b.type == ACT_INTEGER);
    REQUIRE(cf1.v.avm_func->constants[0].integer.val == 0xCF1);
    REQUIRE(cf1.v.avm_func->header->num_imports == 1);
    REQUIRE(cf1.v.avm_func->import_values[0].tag.b == ABT_FUNCTION);
    REQUIRE(cf1.v.avm_func->import_values[0].tag.variant == AVTF_AVM);
    avalue_t cf1i = cf1.v.avm_func->import_values[0];
    REQUIRE(cf1i.tag.b == ABT_FUNCTION);
    REQUIRE(cf1i.tag.variant == AVTF_AVM);
    aprototype_t* cf1ip = cf1i.v.avm_func;
    aprototype_t* cf1icp = cf1ip->chunk->prototypes;
    REQUIRE(strcmp(cf1icp->strings + cf1icp->header->symbol, "mod_a") == 0);
    REQUIRE(strcmp(cf1ip->strings + cf1ip->header->symbol, "f3") == 0);

    // sweep testing
    alist_node_t* old_a = alist_head(&l.garbages);
    REQUIRE(!alist_is_end(&l.garbages, old_a));
    achunk_t* old_a_chunk = ALIST_NODE_CAST(achunk_t, old_a);
    aprototype_t* old_a_chunk_p = old_a_chunk->prototypes;
    REQUIRE(strcmp(
        old_a_chunk_p->strings + old_a_chunk_p->header->symbol,
        "mod_a") == 0);
    old_a_chunk->retain = FALSE;
    aloader_sweep(&l);
    old_a = alist_head(&l.garbages);
    REQUIRE(alist_is_end(&l.garbages, old_a));

    aloader_cleanup(&l);

    aasm_cleanup(&a);
    aasm_cleanup(&aa);
    aasm_cleanup(&b);
    aasm_cleanup(&c);
}

#endif // ANT_TOOL