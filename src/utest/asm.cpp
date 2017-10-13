/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include "prereq.h"

#include <any/asm.h>
#include <any/string_table.h>

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
    ainstruction_t cls;
    ainstruction_t jmp;
    ainstruction_t jin;
    ainstruction_t ivk;
    ainstruction_t snd;
    ainstruction_t rcv;
    ainstruction_t rmv;
    ainstruction_t rwd;
    ainstruction_t add;
    ainstruction_t sub;
    ainstruction_t mul;
    ainstruction_t div;
    ainstruction_t not;
    ainstruction_t ne;
    ainstruction_t eq;
    ainstruction_t lt;
    ainstruction_t le;
    ainstruction_t gt;
    ainstruction_t ge;

    aconstant_t cinteger;
    aconstant_t cstring;
    aconstant_t creal;
} basic_test_ctx;

static void basic_add(aasm_t* a, basic_test_ctx& t)
{
    t.pop = ai_pop(rand());
    t.ldk = ai_ldk(rand());
    t.llv = ai_llv(rand());
    t.slv = ai_slv(rand());
    t.imp = ai_imp(rand());
    t.cls = ai_cls(rand());
    t.jmp = ai_jmp(rand());
    t.jin = ai_jin(rand());
    t.ivk = ai_ivk(rand());
    t.rcv = ai_rcv(rand());

    REQUIRE(0 == aasm_emit(a, ai_nop(), 1));
    REQUIRE(1 == aasm_emit(a, ai_brk(), 2));
    REQUIRE(2 == aasm_emit(a, t.pop, 3));
    REQUIRE(3 == aasm_emit(a, t.ldk, 4));
    REQUIRE(4 == aasm_emit(a, ai_nil(), 5));
    REQUIRE(5 == aasm_emit(a, ai_ldb(TRUE), 6));
    REQUIRE(6 == aasm_emit(a, ai_ldb(FALSE), 7));
    REQUIRE(7 == aasm_emit(a, ai_lsi(0xFEFA), 8));
    REQUIRE(8 == aasm_emit(a, t.llv, 9));
    REQUIRE(9 == aasm_emit(a, t.slv, 10));
    REQUIRE(10 == aasm_emit(a, t.imp, 11));
    REQUIRE(11 == aasm_emit(a, t.cls, 12));
    REQUIRE(12 == aasm_emit(a, t.jmp, 13));
    REQUIRE(13 == aasm_emit(a, t.jin, 14));
    REQUIRE(14 == aasm_emit(a, t.ivk, 15));
    REQUIRE(15 == aasm_emit(a, ai_ret(), 16));
    REQUIRE(16 == aasm_emit(a, ai_snd(), 17));
    REQUIRE(17 == aasm_emit(a, t.rcv, 18));
    REQUIRE(18 == aasm_emit(a, ai_rmv(), 19));
    REQUIRE(19 == aasm_emit(a, ai_rwd(), 20));
    REQUIRE(20 == aasm_emit(a, ai_add(), 21));
    REQUIRE(21 == aasm_emit(a, ai_sub(), 22));
    REQUIRE(22 == aasm_emit(a, ai_mul(), 23));
    REQUIRE(23 == aasm_emit(a, ai_div(), 24));
    REQUIRE(24 == aasm_emit(a, ai_not(), 25));
    REQUIRE(25 == aasm_emit(a, ai_eq(), 26));
    REQUIRE(26 == aasm_emit(a, ai_lt(), 27));
    REQUIRE(27 == aasm_emit(a, ai_le(), 28));
    REQUIRE(28 == aasm_emit(a, ai_gt(), 29));
    REQUIRE(29 == aasm_emit(a, ai_ge(), 30));

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
    aasm_prototype_t* p = aasm_prototype(a);
    aasm_current_t c = aasm_resolve(a);

    num_vs_capacity_check(p);

    REQUIRE(p->num_instructions == 30);
    REQUIRE(c.instructions[0].b.opcode == AOC_NOP);
    REQUIRE(c.source_lines[0] == 1);
    REQUIRE(c.instructions[1].b.opcode == AOC_BRK);
    REQUIRE(c.source_lines[1] == 2);
    REQUIRE(c.instructions[2].b.opcode == AOC_POP);
    REQUIRE(c.instructions[2].pop.n == t.pop.pop.n);
    REQUIRE(c.source_lines[2] == 3);
    REQUIRE(c.instructions[3].b.opcode == AOC_LDK);
    REQUIRE(c.instructions[3].ldk.idx == t.ldk.ldk.idx);
    REQUIRE(c.source_lines[3] == 4);
    REQUIRE(c.instructions[4].b.opcode == AOC_NIL);
    REQUIRE(c.source_lines[4] == 5);
    REQUIRE(c.instructions[5].b.opcode == AOC_LDB);
    REQUIRE(c.instructions[5].ldb.val == TRUE);
    REQUIRE(c.source_lines[5] == 6);
    REQUIRE(c.instructions[6].b.opcode == AOC_LDB);
    REQUIRE(c.instructions[6].ldb.val == FALSE);
    REQUIRE(c.source_lines[6] == 7);
    REQUIRE(c.instructions[7].b.opcode == AOC_LSI);
    REQUIRE(c.instructions[7].lsi.val == 0xFEFA);
    REQUIRE(c.source_lines[7] == 8);
    REQUIRE(c.instructions[8].b.opcode == AOC_LLV);
    REQUIRE(c.instructions[8].llv.idx == t.llv.llv.idx);
    REQUIRE(c.source_lines[8] == 9);
    REQUIRE(c.instructions[9].b.opcode == AOC_SLV);
    REQUIRE(c.instructions[9].slv.idx == t.slv.slv.idx);
    REQUIRE(c.source_lines[9] == 10);
    REQUIRE(c.instructions[10].b.opcode == AOC_IMP);
    REQUIRE(c.instructions[10].imp.idx == t.imp.imp.idx);
    REQUIRE(c.source_lines[10] == 11);
    REQUIRE(c.instructions[11].b.opcode == AOC_CLS);
    REQUIRE(c.instructions[11].cls.idx == t.cls.cls.idx);
    REQUIRE(c.source_lines[11] == 12);
    REQUIRE(c.instructions[12].b.opcode == AOC_JMP);
    REQUIRE(c.instructions[12].jmp.displacement == t.jmp.jmp.displacement);
    REQUIRE(c.source_lines[12] == 13);
    REQUIRE(c.instructions[13].b.opcode == AOC_JIN);
    REQUIRE(c.instructions[13].jin.displacement == t.jin.jin.displacement);
    REQUIRE(c.source_lines[13] == 14);
    REQUIRE(c.instructions[14].b.opcode == AOC_IVK);
    REQUIRE(c.instructions[14].ivk.nargs == t.ivk.ivk.nargs);
    REQUIRE(c.source_lines[14] == 15);
    REQUIRE(c.instructions[15].b.opcode == AOC_RET);
    REQUIRE(c.source_lines[15] == 16);
    REQUIRE(c.instructions[16].b.opcode == AOC_SND);
    REQUIRE(c.source_lines[16] == 17);
    REQUIRE(c.instructions[17].b.opcode == AOC_RCV);
    REQUIRE(c.instructions[17].rcv.displacement == t.rcv.rcv.displacement);
    REQUIRE(c.source_lines[17] == 18);
    REQUIRE(c.instructions[18].b.opcode == AOC_RMV);
    REQUIRE(c.source_lines[18] == 19);
    REQUIRE(c.instructions[19].b.opcode == AOC_RWD);
    REQUIRE(c.source_lines[19] == 20);
    REQUIRE(c.instructions[20].b.opcode == AOC_ADD);
    REQUIRE(c.source_lines[20] == 21);
    REQUIRE(c.instructions[21].b.opcode == AOC_SUB);
    REQUIRE(c.source_lines[21] == 22);
    REQUIRE(c.instructions[22].b.opcode == AOC_MUL);
    REQUIRE(c.source_lines[22] == 23);
    REQUIRE(c.instructions[23].b.opcode == AOC_DIV);
    REQUIRE(c.source_lines[23] == 24);

    REQUIRE(c.instructions[24].b.opcode == AOC_NOT);
    REQUIRE(c.source_lines[24] == 25);
    REQUIRE(c.instructions[25].b.opcode == AOC_EQ);
    REQUIRE(c.source_lines[25] == 26);
    REQUIRE(c.instructions[26].b.opcode == AOC_LT);
    REQUIRE(c.source_lines[26] == 27);
    REQUIRE(c.instructions[27].b.opcode == AOC_LE);
    REQUIRE(c.source_lines[27] == 28);
    REQUIRE(c.instructions[28].b.opcode == AOC_GT);
    REQUIRE(c.source_lines[28] == 29);
    REQUIRE(c.instructions[29].b.opcode == AOC_GE);
    REQUIRE(c.source_lines[29] == 30);

    REQUIRE(p->num_constants == 3);
    REQUIRE(c.constants[0].type == ACT_INTEGER);
    REQUIRE(c.constants[0].integer == t.cinteger.integer);
    REQUIRE(c.constants[1].type == ACT_STRING);
    REQUIRE(c.constants[1].string == t.cstring.string);
    REQUIRE(c.constants[2].type == ACT_REAL);
    REQUIRE(c.constants[2].real == t.creal.real);

    REQUIRE(p->num_imports == 3);
    CHECK_THAT(astring_table_to_string(a->st, c.imports[0].module),
        Catch::Equals("tim0"));
    CHECK_THAT(astring_table_to_string(a->st, c.imports[0].name),
        Catch::Equals("tin0"));
    CHECK_THAT(astring_table_to_string(a->st, c.imports[1].module),
        Catch::Equals("tim1"));
    CHECK_THAT(astring_table_to_string(a->st, c.imports[1].name),
        Catch::Equals("tin1"));
    CHECK_THAT(astring_table_to_string(a->st, c.imports[2].module),
        Catch::Equals("tim2"));
    CHECK_THAT(astring_table_to_string(a->st, c.imports[2].name),
        Catch::Equals("tin2"));
}

static void require_equals(aasm_t* a1, aasm_t* a2)
{
    aasm_prototype_t* p1 = aasm_prototype(a1);
    aasm_prototype_t* p2 = aasm_prototype(a2);
    aasm_current_t c1 = aasm_resolve(a1);
    aasm_current_t c2 = aasm_resolve(a2);

    CHECK_THAT(
        astring_table_to_string(a1->st, p1->source),
        Catch::Equals(astring_table_to_string(a2->st, p2->source)));
    CHECK_THAT(
        astring_table_to_string(a1->st, p1->symbol),
        Catch::Equals(astring_table_to_string(a2->st, p2->symbol)));

    REQUIRE(p1->num_nesteds == p2->num_nesteds);

    REQUIRE(p1->num_instructions == p2->num_instructions);
    REQUIRE(
        memcmp(
            c1.instructions,
            c2.instructions,
            (size_t)p1->num_instructions * sizeof(ainstruction_t)) == 0);

    REQUIRE(p1->num_constants == p2->num_constants);
    for (aint_t i = 0; i < p1->num_constants; ++i) {
        REQUIRE(c1.constants[i].type == c2.constants[i].type);
        switch (c1.constants[i].type) {
        case ACT_INTEGER:
            REQUIRE(c1.constants[i].integer == c2.constants[i].integer);
            break;
        case ACT_STRING:
            CHECK_THAT(
                astring_table_to_string(a1->st, c1.constants[i].string),
                Catch::Equals(
                    astring_table_to_string(a2->st, c2.constants[i].string)));
            break;
        case ACT_REAL:
            REQUIRE(c1.constants[i].real == c2.constants[i].real);
            break;
        }
    }

    REQUIRE(p1->num_imports == p2->num_imports);
    for (aint_t i = 0; i < p2->num_imports; ++i) {
        CHECK_THAT(
            astring_table_to_string(a1->st, c1.imports[i].module),
            Catch::Equals(
                astring_table_to_string(a2->st, c2.imports[i].module)));
        CHECK_THAT(
            astring_table_to_string(a1->st, c1.imports[i].name),
            Catch::Equals(
                astring_table_to_string(a2->st, c2.imports[i].name)));
    }
}

TEST_CASE("asm_module")
{
    aasm_t a;
    aasm_init(&a, &myalloc, NULL);
    REQUIRE(aasm_load(&a, NULL) == AERR_NONE);
    aasm_prototype_t* p = aasm_prototype(&a);

    REQUIRE(p->source == 0);
    REQUIRE(p->symbol == 0);
    REQUIRE(p->num_instructions == 0);
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

    static aint_t PUSH_COUNT = 25;

    for (aint_t i = 0; i < ANY_ASM_MAX_NESTED_LEVEL; ++i) {
        for (aint_t j = 0; j < PUSH_COUNT; ++j) {
            REQUIRE(j == aasm_push(&a));
            basic_test_ctx t;
            basic_add(&a, t);
            basic_check(&a, t);
            REQUIRE(j == aasm_pop(&a));
            aasm_open(&a, j);
            basic_check(&a, t);
            REQUIRE(j == aasm_pop(&a));
        }
        if (i == 0) {
            REQUIRE(PUSH_COUNT == aasm_module_push(&a, "symbol"));
            const aasm_prototype_t* p = aasm_prototype(&a);
            CHECK_THAT(astring_table_to_string(a.st, p->symbol),
                Catch::Equals("symbol"));
        } else {
            REQUIRE(PUSH_COUNT == aasm_push(&a));
        }
    }

    for (aint_t i = ANY_ASM_MAX_NESTED_LEVEL - 1; i >= 0; --i) {
        REQUIRE(PUSH_COUNT == aasm_pop(&a));
    }

    aasm_cleanup(&a);
}

TEST_CASE("asm_save_load")
{
    aasm_t a1;
    aasm_init(&a1, &myalloc, NULL);
    REQUIRE(aasm_load(&a1, NULL) == AERR_NONE);

    static aint_t PUSH_COUNT = 5;

    for (aint_t i = 0; i < ANY_ASM_MAX_NESTED_LEVEL; ++i) {
        for (aint_t j = 0; j < PUSH_COUNT; ++j) {
            REQUIRE(j == aasm_push(&a1));
            basic_test_ctx t;
            basic_add(&a1, t);
            basic_check(&a1, t);
            REQUIRE(j == aasm_pop(&a1));
        }
        REQUIRE(PUSH_COUNT == aasm_push(&a1));
    }

    aasm_save(&a1);

    for (aint_t i = ANY_ASM_MAX_NESTED_LEVEL - 1; i >= 0; --i) {
        REQUIRE(PUSH_COUNT == aasm_pop(&a1));
    }

    aasm_t a2;
    aasm_init(&a2, &myalloc, NULL);
    REQUIRE(aasm_load(&a2, a1.chunk) == AERR_NONE);

    for (aint_t i = 0; i < ANY_ASM_MAX_NESTED_LEVEL; ++i) {
        for (aint_t j = 0; j < PUSH_COUNT; ++j) {
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