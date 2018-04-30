// Copyright (c) 2017-2018 Nguyen Viet Giang. All rights reserved.
#include "prereq.h"
#include <list>
#include <dlist.h>

using namespace std;

typedef list<u8> ListModel;

struct SutNode
{
    anode_t node;
    u8 value;
    SutNode(u8 value) : value(value) { /*nop*/ }
};

static void
same_contents(
    const state::Command<ListModel, anode_t> *m,
    const ListModel &s0, anode_t &sut)
{
    ListModel s1(s0);
    m->apply(s1);
    ListModel::const_iterator s1_i = s1.begin();
    auto sut_i = alist_head(&sut);
    while (s1_i != s1.end()) {
        RC_ASSERT(alist_not_end(&sut, sut_i));
        RC_ASSERT(*s1_i == AFROM_FIELD(SutNode, sut_i, node)->value);
        ++s1_i;
        sut_i = sut_i->next;
    }
    RC_ASSERT(alist_end(&sut, sut_i));
}

struct PushHead : state::Command<ListModel, anode_t>
{
    const u8 v;

    PushHead() : v(*gen::arbitrary<u8>()) { /*nop*/ }

    void apply(ListModel &s0) const override
    {
        s0.push_front(v);
    }

    void run(const ListModel &s0, anode_t &sut) const override
    {
        auto n = new SutNode(v);
        alist_push_head(&sut, &n->node);
        same_contents(this, s0, sut);
    }

    void show(ostream &os) const override
    {
        os << "PushHead(" << (u32)v << ")";
    }
};

struct PushBack : state::Command<ListModel, anode_t>
{
    const u8 v;

    PushBack() : v(*gen::arbitrary<u8>()) { /*nop*/ }

    void apply(ListModel &s0) const override
    {
        s0.push_back(v);
    }

    void run(const ListModel &s0, anode_t &sut) const override
    {
        auto n = new SutNode(v);
        alist_push_back(&sut, &n->node);
        same_contents(this, s0, sut);
    }

    void show(ostream &os) const override
    {
        os << "PushBack(" << (u32)v << ")";
    }
};

struct Unlink : state::Command<ListModel, anode_t>
{
    const int idx;

    explicit Unlink(const ListModel& s0)
        : idx(*gen::inRange(0, (int)s0.size() - 1))
    { /*nop*/ }

    void checkPreconditions(const Model &s0) const override
    {
        RC_PRE(s0.size() > 0);
    }

    void apply(ListModel &s0) const override
    {
        s0.erase(std::next(s0.begin(), idx));
    }

    void run(const ListModel &s0, anode_t &sut) const override
    {
        auto sut_i = alist_head(&sut);
        for (int i = 0; i < idx; ++i) {
            RC_ASSERT(alist_not_end(&sut, sut_i));
            sut_i = sut_i->next;
        }
        RC_ASSERT(alist_not_end(&sut, sut_i));
        anode_unlink(sut_i);
        delete sut_i;
        same_contents(this, s0, sut);
    }

    void show(ostream &os) const override
    {
        os << "Unlink(" << idx << ")";
    }
};

TEST_CASE("dlist")
{
    REQUIRE(check([] {
        ListModel s0;
        anode_t sut;
        alist_init(&sut);
        state::check(s0, sut, state::gen::execOneOfWithArgs<
            PushHead, PushBack, Unlink>());
        auto sut_i = alist_head(&sut);
        while (alist_not_end(&sut, sut_i)) {
            auto next = sut_i->next;
            delete sut_i;
            sut_i = next;
        }
    }));
}