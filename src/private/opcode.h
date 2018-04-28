// Copyright (c) 2017-2018 Nguyen Viet Giang. All rights reserved.
#ifndef _AVM_OPCODE_H_
#define _AVM_OPCODE_H_

#include <avm/prereq.h>

/// Specify the operation to be performed by the instructions.
typedef enum aopcode_e {
    AOC_NOP = 0,
    AOC_BRK = 1,
    AOC_POP = 2,

    AOC_LDK = 10,
    AOC_NIL = 11,
    AOC_LDB = 12,
    AOC_LSI = 13,
    AOC_LLV = 14,
    AOC_SLV = 15,
    AOC_IMP = 16,
    AOC_CLS = 17,

    AOC_JMP = 30,
    AOC_JIN = 31,

    AOC_IVK = 40,
    AOC_RET = 41,

    AOC_ADD = 60,
    AOC_SUB = 61,
    AOC_MUL = 62,
    AOC_DIV = 63
} aopcode_t;

/** Base type.
\rst
======  =======
8 bits  24 bits
======  =======
opcode  payload
======  =======
\endrst
*/
typedef struct ai_base_s {
    s32 opcode : 8;
    s32 _ : 24;
} ai_base_t;

/** No operation.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_NOP  _
=======  =======
\endrst
*/
typedef struct ai_nop_s {
    s32 _;
} ai_nop_t;

/** Suspend the execution until being resumed by debugger.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_BRK  _
=======  =======
\endrst
*/
typedef struct ai_brk_s {
    s32 _;
} ai_brk_t;

/** Pop `n` elements from the stack.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_POP  n
=======  =======
\endrst
*/
typedef struct ai_pop_s {
    s32 _ : 8;
    s32 n : 24;
} ai_pop_t;

/** Push a constant from const pool at `idx` onto the stack.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_LDK  idx
=======  =======
\endrst
*/
typedef struct ai_ldk_s {
    s32 _ : 8;
    s32 idx : 24;
} ai_ldk_t;

/** Push a nil value onto the stack.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_NIL  _
=======  =======
\endrst
*/
typedef struct ai_nil_s {
    s32 _;
} ai_nil_t;

/** Push a bool value onto the stack.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_LDB  value
=======  =======
\endrst
*/
typedef struct ai_ldb_s {
    s32 _ : 8;
    s32 value : 24;
} ai_ldb_t;

/** Push a small integer value (24 bits) onto the stack.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_LSI  value
=======  =======
\endrst
*/
typedef struct ai_lsi_s {
    s32 _ : 8;
    s32 value : 24;
} ai_lsi_t;

/** Push a value at local `slot` onto the stack.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_LLV  slot
=======  =======
\endrst
*/
typedef struct ai_llv_s {
    s32 _ : 8;
    s32 slot : 24;
} ai_llv_t;

/** Pop a value from the stack and set it to the local `slot`.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_SLV  slot
=======  =======
\endrst
*/
typedef struct ai_slv_s {
    s32 _ : 8;
    s32 slot : 24;
} ai_slv_t;

/** Push a value from import pool at `idx` onto the stack.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_IMP  idx
=======  =======
\endrst
*/
typedef struct ai_imp_s {
    s32 _ : 8;
    s32 idx : 24;
} ai_imp_t;

/** Make a closure of prototype at `idx` and push it onto the stack.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_CLS  idx
=======  =======
\endrst
*/
typedef struct ai_cls_s {
    s32 _ : 8;
    s32 idx : 24;
} ai_cls_t;

/** Unconditional jump to relative signed `offset`.
\rst
=======  ============
8 bits   24 bits
=======  ============
AOC_JMP  offset
=======  ============
\endrst
*/
typedef struct ai_jmp_s {
    s32 _ : 8;
    s32 offset : 24;
} ai_jmp_t;

/** Conditional jump to relative signed `offset`.
\brief Pop a boolean value from the stack and jump if isn't true.
\rst
=======  ============
8 bits   24 bits
=======  ============
AOC_JIN  offset
=======  ============
\endrst
*/
typedef struct ai_jin_s {
    s32 _ : 8;
    s32 offset : 24;
} ai_jin_t;

/** Function call.
\brief Please refer \ref avm_call for the calling convention.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_IVK  nargs
=======  =======
\endrst
*/
typedef struct ai_ivk_s {
    s32 _ : 8;
    s32 nargs : 24;
} ai_ivk_t;

/** Returning from a function call.
\brief Please refer \ref avm_call for the calling convention.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_RET  _
=======  =======
\endrst
*/
typedef struct ai_ret_s {
    s32 _;
} ai_ret_t;

/** Add two numbers.
\rst
=======  ============
8 bits   24 bits
=======  ============
AOC_ADD  _
=======  ============
\endrst
*/
typedef struct ai_add_s {
    s32 _;
} ai_add_t;

/** Subtract two numbers, top is subtrahend.
\rst
=======  ============
8 bits   24 bits
=======  ============
AOC_SUB  _
=======  ============
\endrst
*/
typedef struct ai_sub_s {
    s32 _;
} ai_sub_t;

/** Multiply two numbers.
\rst
=======  ============
8 bits   24 bits
=======  ============
AOC_MUL  _
=======  ============
\endrst
*/
typedef struct ai_mul_s {
    s32 _;
} ai_mul_t;

/** Divide two numbers, top is denominator.
\rst
=======  ============
8 bits   24 bits
=======  ============
AOC_DIV  _
=======  ============
\endrst
*/
typedef struct ai_div_s {
    s32 _;
} ai_div_t;

/// Variant of instruction types.
typedef union {
    ai_base_t b;
    ai_pop_t pop;
    ai_ldk_t ldk;
    ai_ldb_t ldb;
    ai_lsi_t lsi;
    ai_llv_t llv;
    ai_slv_t slv;
    ai_imp_t imp;
    ai_cls_t cls;
    ai_jmp_t jmp;
    ai_jin_t jin;
    ai_ivk_t ivk;
} ainstruction_t;
AALIGNAS(ainstruction_t, 4);

// Instruction size is fixed 4 bytes.
ASTATIC_ASSERT(sizeof(ainstruction_t) == 4);

#endif // !_AVM_OPCODE_H_