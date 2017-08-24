/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/actor.h>

#include <any/std_string.h>

void actor_dispatch(aactor_t* a)
{
    aframe_t* frame = a->frame;
    aprototype_t* pt = frame->pt;
    aprototype_header_t* pth = pt->header;
    for (; frame->ip < pth->num_instructions; ++frame->ip) {
        ainstruction_t* i = pt->instructions + frame->ip;
        switch (i->b.opcode) {
        case AOC_NOP:
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
            avalue_t v;
            any_pop(a, 1);
            v = a->stack.v[a->stack.sp];
            if (v.tag.type != AVT_BOOLEAN && v.tag.type != AVT_NIL) {
                any_error(a, AERR_RUNTIME, "condition must be boolean or nil");
            }
            if (v.tag.type != AVT_NIL && v.v.boolean) continue;
            goto jmp;
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
            avalue_t timeout = a->stack.v[a->stack.sp - 1];
            if (timeout.tag.type != AVT_INTEGER) {
                any_error(a, AERR_RUNTIME, "timeout must be integer");
            } else {
                if (any_mbox_recv(a, timeout.v.integer) == AERR_TIMEOUT) {
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
        default:
            any_error(a, AERR_RUNTIME, "bad instruction %u", i->b.opcode);
            break;
        }
    }
    any_error(a, AERR_RUNTIME, "return missing");
}