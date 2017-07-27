/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#ifdef ANY_TOOL

#include <catch.hpp>

#include <stdlib.h>
#include <string.h>
#include <any/asm.h>
#include <any/errno.h>
#include <any/string_table.h>

#define REQUIRE_STR_EQUALS(a, b) REQUIRE(strcmp(a, b) == 0)

static void* myalloc(void*, void* old, int32_t sz)
{
    return realloc(old, sz);
}

static void num_vs_capacity_check(aasm_prototype_t* p)
{
    REQUIRE(p->num_instructions <= p->max_instructions);
    REQUIRE(p->num_constants <= p->max_constants);
    REQUIRE(p->num_imports <= p->max_imports);
    REQUIRE(p->num_nesteds <= p->max_nesteds);
}

typedef struct {
    ainstruction_t pop;
    ainstruction_t ldk;
    ainstruction_t llv;
    ainstruction_t slv;
    ainstruction_t imp;
    ainstruction_t jmp;
    ainstruction_t jin;
    ainstruction_t ivk;

    aasm_constant_t cinteger;
    aasm_constant_t cstring;
    aasm_constant_t creal;
} basic_test_ctx;

static void basic_add(aasm_t* a, basic_test_ctx& t)
{
    t.pop = ai_pop(rand());
    t.ldk = ai_ldk(rand());
    t.llv = ai_llv(rand());
    t.slv = ai_slv(rand());
    t.imp = ai_imp(rand());
    t.jmp = ai_jmp(rand());
    t.jin = ai_jin(rand());
    t.ivk = ai_ivk(rand());

    REQUIRE(0 == aasm_emit(a, ai_nop()));
    REQUIRE(1 == aasm_emit(a, t.pop));
    REQUIRE(2 == aasm_emit(a, t.ldk));
    REQUIRE(3 == aasm_emit(a, ai_nil()));
    REQUIRE(4 == aasm_emit(a, ai_ldb(TRUE)));
    REQUIRE(5 == aasm_emit(a, ai_ldb(FALSE)));
    REQUIRE(6 == aasm_emit(a, t.llv));
    REQUIRE(7 == aasm_emit(a, t.slv));
    REQUIRE(8 == aasm_emit(a, t.imp));
    REQUIRE(9 == aasm_emit(a, t.jmp));
    REQUIRE(10 == aasm_emit(a, t.jin));
    REQUIRE(11 == aasm_emit(a, t.ivk));
    REQUIRE(12 == aasm_emit(a, ai_ret()));

    t.cinteger = ac_integer(rand());
    t.cstring = ac_string(aasm_string_to_ref(a, "test_const"));
    t.creal = ac_real((areal_t)rand());

    REQUIRE(0 == aasm_add_constant(a, t.cinteger));
    REQUIRE(1 == aasm_add_constant(a, t.cstring));
    REQUIRE(2 == aasm_add_constant(a, t.creal));

    REQUIRE(0 == aasm_add_import(a, "tim0", "tin0"));
    REQUIRE(1 == aasm_add_import(a, "tim1", "tin1"));
    REQUIRE(2 == aasm_add_import(a, "tim2", "tin2"));
};

static void basic_check(aasm_t* a, basic_test_ctx& t)
{
    auto p = aasm_prototype(a);
    auto c = aasm_resolve(a);

    num_vs_capacity_check(p);

    REQUIRE(p->num_instructions == 13);
    REQUIRE(c.instructions[0].b.opcode == AOC_NOP);
    REQUIRE(c.instructions[1].b.opcode == AOC_POP);
    REQUIRE(c.instructions[1].pop.n == t.pop.pop.n);
    REQUIRE(c.instructions[2].b.opcode == AOC_LDK);
    REQUIRE(c.instructions[2].ldk.idx == t.ldk.ldk.idx);
    REQUIRE(c.instructions[3].b.opcode == AOC_NIL);
    REQUIRE(c.instructions[4].b.opcode == AOC_LDB);
    REQUIRE(c.instructions[4].ldb.val == TRUE);
    REQUIRE(c.instructions[5].b.opcode == AOC_LDB);
    REQUIRE(c.instructions[5].ldb.val == FALSE);
    REQUIRE(c.instructions[6].b.opcode == AOC_LLV);
    REQUIRE(c.instructions[6].llv.idx == t.llv.llv.idx);
    REQUIRE(c.instructions[7].b.opcode == AOC_SLV);
    REQUIRE(c.instructions[7].slv.idx == t.slv.slv.idx);
    REQUIRE(c.instructions[8].b.opcode == AOC_IMP);
    REQUIRE(c.instructions[8].imp.idx == t.imp.imp.idx);
    REQUIRE(c.instructions[9].b.opcode == AOC_JMP);
    REQUIRE(c.instructions[9].jmp.displacement == t.jmp.jmp.displacement);
    REQUIRE(c.instructions[10].b.opcode == AOC_JIN);
    REQUIRE(c.instructions[10].jin.displacement == t.jin.jin.displacement);
    REQUIRE(c.instructions[11].b.opcode == AOC_IVK);
    REQUIRE(c.instructions[11].ivk.nargs == t.ivk.ivk.nargs);
    REQUIRE(c.instructions[12].b.opcode == AOC_RET);

    REQUIRE(p->num_constants == 3);
    REQUIRE(c.constants[0].b.type == ACT_INTEGER);
    REQUIRE(c.constants[0].integer.val == t.cinteger.integer.val);
    REQUIRE(c.constants[1].b.type == ACT_STRING);
    REQUIRE(c.constants[1].string.ref == t.cstring.string.ref);
    REQUIRE(c.constants[2].b.type == ACT_REAL);
    REQUIRE(c.constants[2].real.val == t.creal.real.val);

    REQUIRE(p->num_imports == 3);
    REQUIRE_STR_EQUALS(astring_table_to_string(a->st, c.imports[0].module), "tim0");
    REQUIRE_STR_EQUALS(astring_table_to_string(a->st, c.imports[0].name), "tin0");
    REQUIRE_STR_EQUALS(astring_table_to_string(a->st, c.imports[1].module), "tim1");
    REQUIRE_STR_EQUALS(astring_table_to_string(a->st, c.imports[1].name), "tin1");
    REQUIRE_STR_EQUALS(astring_table_to_string(a->st, c.imports[2].module), "tim2");
    REQUIRE_STR_EQUALS(astring_table_to_string(a->st, c.imports[2].name), "tin2");
}

static void require_equals(aasm_t* a1, aasm_t* a2)
{
    auto p1 = aasm_prototype(a1);
    auto p2 = aasm_prototype(a2);
    auto c1 = aasm_resolve(a1);
    auto c2 = aasm_resolve(a2);

    REQUIRE_STR_EQUALS(
        astring_table_to_string(a1->st, p1->source),
        astring_table_to_string(a2->st, p2->source));
    REQUIRE_STR_EQUALS(
        astring_table_to_string(a1->st, p1->symbol),
        astring_table_to_string(a2->st, p2->symbol));

    REQUIRE(p1->num_upvalues == p2->num_upvalues);
    REQUIRE(p1->num_arguments == p2->num_arguments);
    REQUIRE(p1->num_local_vars == p2->num_local_vars);
    REQUIRE(p1->num_nesteds == p2->num_nesteds);

    REQUIRE(p1->num_instructions == p2->num_instructions);
    REQUIRE(
        memcmp(
            c1.instructions,
            c2.instructions,
            p1->num_instructions*sizeof(ainstruction_t)) == 0);

    REQUIRE(p1->num_constants == p2->num_constants);
    for (int32_t integer = 0; integer < p1->num_constants; ++integer) {
        REQUIRE(c1.constants[integer].b.type == c2.constants[integer].b.type);
        switch (c1.constants[integer].b.type) {
        case ACT_INTEGER:
            REQUIRE(c1.constants[integer].integer.val == c2.constants[integer].integer.val);
            break;
        case ACT_STRING:
            REQUIRE_STR_EQUALS(
                astring_table_to_string(a1->st, c1.constants[integer].string.ref),
                astring_table_to_string(a2->st, c2.constants[integer].string.ref));
            break;
        case ACT_REAL:
            REQUIRE(c1.constants[integer].real.val == c2.constants[integer].real.val);
            break;
        }
    }

    REQUIRE(p1->num_imports == p2->num_imports);
    for (int32_t integer = 0; integer < p2->num_imports; ++integer) {
        REQUIRE_STR_EQUALS(
            astring_table_to_string(a1->st, c1.imports[integer].module),
            astring_table_to_string(a2->st, c2.imports[integer].module));
        REQUIRE_STR_EQUALS(
            astring_table_to_string(a1->st, c1.imports[integer].name),
            astring_table_to_string(a2->st, c2.imports[integer].name));
    }
}

TEST_CASE("asm_module")
{
    aasm_t a;
    aasm_init(&a, &myalloc, NULL);
    REQUIRE(aasm_load(&a, NULL) == AERR_NONE);
    auto p = aasm_prototype(&a);

    REQUIRE(p->source == 0);
    REQUIRE(p->symbol == 0);
    REQUIRE(p->num_instructions == 0);
    REQUIRE(p->num_upvalues == 0);
    REQUIRE(p->num_arguments == 0);
    REQUIRE(p->num_local_vars == 0);
    REQUIRE(p->num_constants == 0);
    REQUIRE(p->num_imports == 0);
    REQUIRE(p->num_nesteds == 0);

    num_vs_capacity_check(p);

    aasm_cleanup(&a);
}

TEST_CASE("asm_basic")
{
    aasm_t a;
    aasm_init(&a, &myalloc, NULL);
    REQUIRE(aasm_load(&a, NULL) == AERR_NONE);

    basic_test_ctx t;
    basic_add(&a, t);
    basic_check(&a, t);

    aasm_cleanup(&a);
}

TEST_CASE("asm_nested")
{
    aasm_t a;
    aasm_init(&a, &myalloc, NULL);
    REQUIRE(aasm_load(&a, NULL) == AERR_NONE);

    enum { PUSH_COUNT = 25 };

    for (int32_t integer = 0; integer < ANY_ASM_MAX_NESTED_LEVEL; ++integer) {
        for (int32_t j = 0; j < PUSH_COUNT; ++j) {
            REQUIRE(j == aasm_push(&a));
            basic_test_ctx t;
            basic_add(&a, t);
            basic_check(&a, t);
            REQUIRE(j == aasm_pop(&a));
            aasm_open(&a, j);
            basic_check(&a, t);
            REQUIRE(j == aasm_pop(&a));
        }
        if (integer == 0) {
            REQUIRE(PUSH_COUNT == aasm_module_push(&a, "symbol"));
            const aasm_prototype_t* p = aasm_prototype(&a);
            REQUIRE_STR_EQUALS(astring_table_to_string(a.st, p->symbol), "symbol");
        } else {
            REQUIRE(PUSH_COUNT == aasm_push(&a));
        }
    }

    for (int32_t integer = ANY_ASM_MAX_NESTED_LEVEL - 1; integer >= 0; --integer) {
        REQUIRE(PUSH_COUNT == aasm_pop(&a));
    }

    aasm_cleanup(&a);
}

TEST_CASE("asm_save_load")
{
    aasm_t a1;
    aasm_init(&a1, &myalloc, NULL);
    REQUIRE(aasm_load(&a1, NULL) == AERR_NONE);

    enum { PUSH_COUNT = 5 };

    for (int32_t integer = 0; integer < ANY_ASM_MAX_NESTED_LEVEL; ++integer) {
        for (int32_t j = 0; j < PUSH_COUNT; ++j) {
            REQUIRE(j == aasm_push(&a1));
            basic_test_ctx t;
            basic_add(&a1, t);
            basic_check(&a1, t);
            REQUIRE(j == aasm_pop(&a1));
        }
        REQUIRE(PUSH_COUNT == aasm_push(&a1));
    }

    aasm_save(&a1);

    for (int32_t integer = ANY_ASM_MAX_NESTED_LEVEL - 1; integer >= 0; --integer) {
        REQUIRE(PUSH_COUNT == aasm_pop(&a1));
    }

    aasm_t a2;
    aasm_init(&a2, &myalloc, NULL);
    REQUIRE(aasm_load(&a2, a1.chunk) == AERR_NONE);

    for (int32_t integer = 0; integer < ANY_ASM_MAX_NESTED_LEVEL; ++integer) {
        for (int32_t j = 0; j < PUSH_COUNT; ++j) {
            aasm_open(&a1, j);
            aasm_open(&a2, j);
            require_equals(&a1, &a2);
            REQUIRE(j == aasm_pop(&a1));
            REQUIRE(j == aasm_pop(&a2));
        }
        aasm_open(&a1, PUSH_COUNT);
        aasm_open(&a2, PUSH_COUNT);
    }

    aasm_cleanup(&a1);
    aasm_cleanup(&a2);
}

#endif // ANY_TOOL