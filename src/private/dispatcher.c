/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/dispatcher.h>

#include <any/process.h>
#include <any/gc_string.h>

aerror_t adispatcher_init(adispatcher_t* self, aprocess_t* owner)
{
    self->owner = owner;
    return AERR_NONE;
}

void adispatcher_call(adispatcher_t* self)
{
    aprocess_t* p = self->owner;
    aframe_t* frame = p->frame;
    aprototype_t* pt = frame->pt;
    aprototype_header_t* pth = pt->header;
    for (; frame->ip < pth->num_instructions; ++frame->ip) {
        ainstruction_t* i = pt->instructions + frame->ip;
        switch (i->b.opcode) {
        case AOC_NOP:
            break;
        case AOC_POP:
            any_pop(p, i->pop.n);
            break;
        case AOC_LDK: {
                aconstant_t* c = pt->constants + i->ldk.idx;
                if (i->ldk.idx < 0 || i->ldk.idx >= pth->num_constants) {
                    any_error(p, AERR_RUNTIME,
                        "bad constant index %d", i->ldk.idx);
                }
                switch (pt->constants[i->ldk.idx].type) {
                case ACT_INTEGER:
                    any_push_integer(p, c->integer);
                    break;
                case ACT_STRING:
                    any_push_string(p, pt->strings + c->string);
                    break;
                case ACT_REAL:
                    any_push_real(p, c->real);
                    break;
                default:
                    any_error(p, AERR_RUNTIME, "bad constant type");
                    break;
                }
            }
            break;
        case AOC_NIL:
            any_push_nil(p);
            break;
        case AOC_LDB:
            any_push_bool(p, i->ldb.val ? TRUE : FALSE);
            break;
        case AOC_LSI:
            any_push_integer(p, i->lsi.val);
            break;
        case AOC_LLV:
            any_push_idx(p, i->llv.idx);
            break;
        case AOC_SLV:
            any_insert(p, i->slv.idx);
            break;
        case AOC_IMP:
            if (i->imp.idx < 0 || i->imp.idx >= pth->num_imports) {
                any_error(p, AERR_RUNTIME, "bad import index %d", i->imp.idx);
            } else {
                aprocess_push(p, pt->import_values + i->imp.idx);
            }
            break;
        case AOC_MKC:
            if (i->mkc.idx < 0 || i->mkc.idx >= pth->num_nesteds) {
                any_error(p, AERR_RUNTIME, "bad nested index %d", i->mkc.idx);
            } else {
                avalue_t v;
                v.tag.b = ABT_FUNCTION;
                v.tag.variant = AVTF_AVM;
                v.v.avm_func = pt->nesteds + i->mkc.idx;
                aprocess_push(p, &v);
            }
            break;
        case AOC_JMP: {
                int32_t nip = frame->ip + i->jmp.displacement + 1;
                if (nip < 0 || nip >= pth->num_instructions) {
                    any_error(p, AERR_RUNTIME, "bad jump");
                } else {
                    frame->ip = nip - 1;
                }
            }
            break;
        case AOC_JIN: {
                int32_t nip;
                any_pop(p, 1);
                if (p->stack[p->sp].tag.b != ABT_BOOL) {
                    any_error(p, AERR_RUNTIME, "condition must be boolean");
                }
                if (p->stack[p->sp].v.boolean) continue;
                nip = frame->ip + i->jin.displacement + 1;
                if (nip < 0 || nip >= pth->num_instructions) {
                    any_error(p, AERR_RUNTIME, "bad jump");
                } else {
                    frame->ip = nip - 1;
                }
            }
            break;
        case AOC_IVK:
            any_call(p, i->ivk.nargs);
            break;
        case AOC_RET:
            return;
        case AOC_SND:
        case AOC_RCV:
        case AOC_RMV:
            any_error(p, AERR_RUNTIME, "TODO");
            break;
        }
    }
    any_error(p, AERR_RUNTIME, "return missing");
}