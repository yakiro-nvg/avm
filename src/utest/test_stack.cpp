// Copyright (c) 2017-2018 Nguyen Viet Giang. All rights reserved.
#include "prereq.h"
#include <vector>
#include <stack.h>

ASTACK_DCL(u8, u8)
ASTACK_DEF(u8, u8)

using namespace std;

typedef vector<u8> StackModel;

struct StackSut
{
    StubAllocator &a;
    au8_stack_t stack;
    StackSut(StubAllocator &a) : a(a) { /*nop*/ }
};

static void
same_contents(
    const state::Command<StackModel, StackSut> *m,
    const StackModel &s0, StackSut &sut)
{
    StackModel s1(s0);
    m->apply(s1);
    RC_ASSERT(s1.size() == sut.stack.count);
    RC_ASSERT(memcmp(
        s1.data(), sut.stack.items, sizeof(u8)*s1.size()) == 0);
}

struct Cleanup : state::Command<StackModel, StackSut>
{
    void apply(StackModel &s0) const override
    {
        s0.clear();
    }

    void run(const StackModel &s0, StackSut &sut) const override
    {
        au8_stack_cleanup(&sut.stack);
        RC_ASSERT(sut.stack.capacity == 0);
        RC_ASSERT(sut.stack.count == 0);
        same_contents(this, s0, sut);
    }

    void show(ostream &os) const override
    {
        os << "Cleanup";
    }
};

struct Shrink : state::Command<StackModel, StackSut>
{
    void run(const StackModel &s0, StackSut &sut) const override
    {
        sut.a.full = false;
        const u32 old_cap = sut.stack.capacity;
        au8_stack_shrink(&sut.stack);
        RC_ASSERT(sut.stack.capacity <= old_cap);
        RC_ASSERT(sut.stack.capacity >= sut.stack.count);
        same_contents(this, s0, sut);
    }

    void show(ostream &os) const override
    {
        os << "Shrink";
    }
};

struct Realloc : state::Command<StackModel, StackSut>
{
    const u32 n;
    const bool full;

    Realloc()
        : n(*gen::inRange<u32>(0u, 10u))
        , full(*gen::arbitrary<bool>())
    { /*nop*/ }

    void apply(StackModel &s0) const override
    {
        if (!full && n < s0.size()) {
            s0.resize(n);
        }
    }

    void run(const StackModel &s0, StackSut &sut) const override
    {
        sut.a.full = full;
        if (full && sut.stack.capacity > sut.stack.count) return;
        const u32 old_cap = sut.stack.capacity;
        const u32 new_count = min(n, sut.stack.count);
        if (ASUCCESS(au8_stack_realloc(&sut.stack, n))) {
            RC_ASSERT(sut.stack.capacity >= n);
            RC_ASSERT(sut.stack.count == new_count);
        } else {
            RC_ASSERT(sut.stack.capacity == old_cap);
        }
        same_contents(this, s0, sut);
    }

    void show(ostream &os) const override
    {
        os << "Realloc(" << n << ", full=" << full << ")";
    }
};

struct Reserve : state::Command<StackModel, StackSut>
{
    const u32 n;
    const bool full;

    Reserve()
        : n(*gen::inRange<u32>(0u, 10u))
        , full(*gen::arbitrary<bool>())
    { /*nop*/ }

    void run(const StackModel &s0, StackSut &sut) const override
    {
        sut.a.full = full;
        const u32 old_cap = sut.stack.capacity;
        if (ASUCCESS(au8_stack_reserve(&sut.stack, n))) {
            RC_ASSERT(sut.stack.count + n <= sut.stack.capacity);
        } else {
            RC_ASSERT(sut.stack.capacity == old_cap);
        }
        same_contents(this, s0, sut);
    }

    void show(ostream &os) const override
    {
        os << "Reserve(" << n << ", full=" << full << ")";
    }
};

struct Push : state::Command<StackModel, StackSut>
{
    const u8 v;
    const bool full;

    Push()
        : v(*gen::arbitrary<u8>())
        , full(*gen::arbitrary<bool>())
    { /*nop*/ }

    void apply(StackModel &s0) const override
    {
        if (!full) s0.push_back(v);
    }

    void run(const StackModel &s0, StackSut &sut) const override
    {
        sut.a.full = full;
        if (full && sut.stack.capacity > sut.stack.count) return;
        au8_stack_push(&sut.stack, &v);
        same_contents(this, s0, sut);
    }

    void show(ostream &os) const override
    {
        os << "Push(" << (u32)v << ", full=" << full << ")";
    }
};

struct Fill : state::Command<StackModel, StackSut>
{
    const u32 n;
    const u8 v;
    const bool full;

    Fill()
        : n(*gen::inRange<u32>(0u, 10u))
        , v(*gen::arbitrary<u8>())
        , full(*gen::arbitrary<bool>())
    { /*nop*/ }

    void apply(StackModel &s0) const override
    {
        if (!full) fill_n(back_inserter(s0), n, v);
    }

    void run(const StackModel &s0, StackSut &sut) const override
    {
        sut.a.full = full;
        if (full && sut.stack.capacity > sut.stack.count) return;
        au8_stack_fill(&sut.stack, &v, n);
        same_contents(this, s0, sut);
    }

    void show(ostream &os) const override
    {
        os << "Fill(v=" << (u32)v << ", n=" << n << ", full=" << full << ")";
    }
};

TEST_CASE("stack")
{
    REQUIRE(check([] {
        StubAllocator a;
        StackModel s0;
        StackSut sut(a);
        RC_SUCCESS(au8_stack_init(&sut.stack, &a, 2));
        RC_ASSERT(sut.stack.capacity >= 2u);
        state::check(s0, sut, state::gen::execOneOfWithArgs<
            Cleanup, Shrink, Realloc, Reserve, Push, Fill>());
        au8_stack_cleanup(&sut.stack);
        a.release();
    }));
}