/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/dispatcher.h>

#include <any/process.h>

aerror_t adispatcher_init(adispatcher_t* self, aprocess_t* owner)
{
    self->owner = owner;
    return AERR_NONE;
}

void adispatcher_call(adispatcher_t* self)
{
    aprocess_t* p = self->owner;
    aframe_t* frame = p->frame;
    for (; frame->ip < frame->pt->header->num_instructions; ++frame->ip) {
        ainstruction_t* i = frame->pt->instructions + frame->ip;
        switch (i->b.opcode) {
        case AOC_NOP:
            break;
        case AOC_POP:
        case AOC_LDK:
        case AOC_NIL:
        case AOC_LDB:
        case AOC_LLV:
        case AOC_SLV:
        case AOC_IMP:
        case AOC_JMP:
        case AOC_JIN:
        case AOC_IVK:
        case AOC_RET:
        case AOC_SND:
        case AOC_RCV:
        case AOC_RMV:
            any_error(p, AERR_RUNTIME, "TODO");
            break;
        }
    }
    any_error(p, AERR_RUNTIME, "return missing");
}