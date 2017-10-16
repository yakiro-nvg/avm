/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/actor.h>

#include <any/std_string.h>
#include <any/std.h>

static void
fill_nil(
    aactor_t* a, aint_t num)
{
    aint_t i;
    for (i = 0; i < num; ++i) {
        any_push_nil(a);
    }
}

#define LOGICAL_OP(op) \
    aint_t cnt = any_count(a); \
    if (cnt < 2) { \
        any_error(a, AERR_RUNTIME, "pop underflow"); \
    } else { \
        aint_t a_lhs = any_check_index(a, cnt - 1); \
        aint_t a_rhs = any_check_index(a, cnt - 2); \
        avalue_t* lhsv = aactor_at(a, a_lhs); \
        avalue_t* rhsv = aactor_at(a, a_rhs); \
        if (lhsv->tag.type == AVT_INTEGER && \
            rhsv->tag.type == AVT_INTEGER) { \
            av_boolean(rhsv, lhsv->v.integer op rhsv->v.integer); \
        } else { \
            areal_t lhs = any_check_real(a, a_lhs); \
            areal_t rhs = any_check_real(a, a_rhs); \
            av_boolean(rhsv, lhs op rhs); \
        } \
        a->stack.sp -= 1; \
    }

void
actor_dispatch(
    aactor_t* a)
{
    aframe_t* frame = a->frame;
    aprototype_t* pt = frame->pt;
    aprototype_header_t* pth = pt->header;
    fill_nil(a, pth->num_local_vars);
    for (; frame->ip < pth->num_instructions; ++frame->ip) {
        ainstruction_t* i = pt->instructions + frame->ip;
        if (a->owner->on_step) {
            while (a->owner->on_step(a, a->owner->on_step_ud) == FALSE) {
                any_yield(a);
            }
        }
        switch (i->b.opcode) {
        case AOC_NOP:
        case AOC_BRK:
            // nop
            break;
        case AOC_POP:
            any_pop(a, i->pop.n);
            break;
        case AOC_LDK: {
            aconstant_t* c = pt->constants + i->ldk.idx;
            if (i->ldk.idx < 0 || i->ldk.idx >= pth->num_constants) {
                any_error(a, AERR_RUNTIME,
                    "bad constant index %d", i->ldk.idx);
            }
            switch (pt->constants[i->ldk.idx].type) {
            case ACT_INTEGER:
                any_push_integer(a, c->integer);
                break;
            case ACT_STRING:
                any_push_string(a, pt->strings + c->string);
                break;
            case ACT_REAL:
                any_push_real(a, c->real);
                break;
            default:
                any_error(a, AERR_RUNTIME, "bad constant type");
                break;
            }
            break;
        }
        case AOC_NIL:
            any_push_nil(a);
            break;
        case AOC_LDB:
            any_push_bool(a, i->ldb.val ? TRUE : FALSE);
            break;
        case AOC_LSI:
            any_push_integer(a, i->lsi.val);
            break;
        case AOC_LLV:
            any_push_index(a, any_check_index(a, i->llv.idx));
            break;
        case AOC_SLV:
            any_insert(a, any_check_index(a, i->slv.idx));
            break;
        case AOC_IMP:
            if (i->imp.idx < 0 || i->imp.idx >= pth->num_imports) {
                any_error(a, AERR_RUNTIME, "bad import index %d", i->imp.idx);
            } else {
                aactor_push(a, pt->import_values + i->imp.idx);
            }
            break;
        case AOC_CLS:
            if (i->cls.idx < 0 || i->cls.idx >= pth->num_nesteds) {
                any_error(a, AERR_RUNTIME, "bad nested index %d", i->cls.idx);
            } else {
                avalue_t v;
                av_byte_code_func(&v, pt->nesteds + i->cls.idx);
                aactor_push(a, &v);
            }
            break;
jmp:
        case AOC_JMP: {
            aint_t nip = frame->ip + i->jmp.displacement + 1;
            if (nip < 0 || nip >= pth->num_instructions) {
                any_error(a, AERR_RUNTIME, "bad jump");
            } else {
                frame->ip = nip - 1;
            }
            break;
        }
        case AOC_JIN: {
            aint_t cnt = any_count(a);
            if (cnt < 1) {
                any_error(a, AERR_RUNTIME, "pop underflow");
            } else {
                aint_t a_cond = any_check_index(a, cnt - 1);
                int32_t cond = any_to_bool(a, a_cond);
                a->stack.sp -= 1;
                if (cond == FALSE) {
                    goto jmp;
                }
            }
            break;
        }
        case AOC_IVK:
            any_call(a, i->ivk.nargs);
            break;
        case AOC_RET:
            return;
        case AOC_SND:
            any_mbox_send(a);
            break;
        case AOC_RCV: {
            aint_t cnt = any_count(a);
            if (cnt < 1) {
                any_error(a, AERR_RUNTIME, "pop underflow");
            } else {
                aint_t a_timeout = any_check_index(a, cnt - 1);
                aint_t timeout = any_check_integer(a, a_timeout);
                if (any_mbox_recv(a, timeout) == AERR_TIMEOUT) {
                    goto jmp;
                }
            }
            break;
        }
        case AOC_RMV:
            any_mbox_remove(a);
            break;
        case AOC_RWD:
            any_mbox_rewind(a);
            break;
        case AOC_ADD: {
            aint_t cnt = any_count(a);
            if (cnt < 2) {
                any_error(a, AERR_RUNTIME, "pop underflow");
            } else {
                aint_t a_lhs = any_check_index(a, cnt - 1);
                aint_t a_rhs = any_check_index(a, cnt - 2);
                avalue_t* lhsv = aactor_at(a, a_lhs);
                avalue_t* rhsv = aactor_at(a, a_rhs);
                if (lhsv->tag.type == AVT_INTEGER &&
                    rhsv->tag.type == AVT_INTEGER) {
                    av_integer(rhsv, lhsv->v.integer + rhsv->v.integer);
                } else {
                    areal_t lhs = any_check_real(a, a_lhs);
                    areal_t rhs = any_check_real(a, a_rhs);
                    av_real(rhsv, lhs + rhs);
                }
                a->stack.sp -= 1;
            }
            break;
        }
        case AOC_SUB: {
            aint_t cnt = any_count(a);
            if (cnt < 2) {
                any_error(a, AERR_RUNTIME, "pop underflow");
            } else {
                aint_t a_lhs = any_check_index(a, cnt - 1);
                aint_t a_rhs = any_check_index(a, cnt - 2);
                avalue_t* lhsv = aactor_at(a, a_lhs);
                avalue_t* rhsv = aactor_at(a, a_rhs);
                if (lhsv->tag.type == AVT_INTEGER &&
                    rhsv->tag.type == AVT_INTEGER) {
                    av_integer(rhsv, lhsv->v.integer - rhsv->v.integer);
                } else {
                    areal_t lhs = any_check_real(a, a_lhs);
                    areal_t rhs = any_check_real(a, a_rhs);
                    av_real(rhsv, lhs - rhs);
                }
                a->stack.sp -= 1;
            }
            break;
        }
        case AOC_MUL: {
            aint_t cnt = any_count(a);
            if (cnt < 2) {
                any_error(a, AERR_RUNTIME, "pop underflow");
            } else {
                aint_t a_lhs = any_check_index(a, cnt - 1);
                aint_t a_rhs = any_check_index(a, cnt - 2);
                avalue_t* lhsv = aactor_at(a, a_lhs);
                avalue_t* rhsv = aactor_at(a, a_rhs);
                if (lhsv->tag.type == AVT_INTEGER &&
                    rhsv->tag.type == AVT_INTEGER) {
                    av_integer(rhsv, lhsv->v.integer * rhsv->v.integer);
                } else {
                    areal_t lhs = any_check_real(a, a_lhs);
                    areal_t rhs = any_check_real(a, a_rhs);
                    av_real(rhsv, lhs * rhs);
                }
                a->stack.sp -= 1;
            }
            break;
        }
        case AOC_DIV: {
            aint_t cnt = any_count(a);
            if (cnt < 2) {
                any_error(a, AERR_RUNTIME, "pop underflow");
            } else {
                aint_t a_lhs = any_check_index(a, cnt - 1);
                aint_t a_rhs = any_check_index(a, cnt - 2);
                avalue_t* lhsv = aactor_at(a, a_lhs);
                avalue_t* rhsv = aactor_at(a, a_rhs);
                if (lhsv->tag.type == AVT_INTEGER &&
                    rhsv->tag.type == AVT_INTEGER) {
                    if (rhsv->v.integer == 0) {
                        any_error(a, AERR_RUNTIME, "divide by zero");
                    }
                    av_integer(rhsv, lhsv->v.integer / rhsv->v.integer);
                } else {
                    areal_t lhs = any_check_real(a, a_lhs);
                    areal_t rhs = any_check_real(a, a_rhs);
                    if (afuzzy_equals(rhs, 0)) {
                        any_error(a, AERR_RUNTIME, "divide by zero");
                    }
                    av_real(rhsv, lhs / rhs);
                }
                a->stack.sp -= 1;
            }
            break;
        }
        case AOC_NOT: {
            aint_t cnt = any_count(a);
            if (cnt < 1) {
                any_error(a, AERR_RUNTIME, "pop underflow");
            } else {
                aint_t a_cond = any_check_index(a, cnt - 1);
                avalue_t* cond = aactor_at(a, a_cond);
                av_boolean(cond, !any_to_bool(a, a_cond));
            }
            break;
        }
        case AOC_EQ: {
            aint_t cnt = any_count(a);
            if (cnt < 2) {
                any_error(a, AERR_RUNTIME, "pop underflow");
            } else {
                aint_t a_lhs = any_check_index(a, cnt - 1);
                aint_t a_rhs = any_check_index(a, cnt - 2);
                avalue_t* rhs = aactor_at(a, a_rhs);
                av_boolean(rhs, any_equals(a, a_lhs, a_rhs));
                a->stack.sp -= 1;
            }
            break;
        }
        case AOC_LT: {
            LOGICAL_OP(<)
            break;
        }
        case AOC_LE: {
            LOGICAL_OP(<=)
            break;
        }
        case AOC_GT: {
            LOGICAL_OP(>)
            break;
        }
        case AOC_GE: {
            LOGICAL_OP(>=)
            break;
        }
        default:
            any_error(a, AERR_RUNTIME, "bad instruction %u", i->b.opcode);
            break;
        }
    }
    any_error(a, AERR_RUNTIME, "return missing");
}