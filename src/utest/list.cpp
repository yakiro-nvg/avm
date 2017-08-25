/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include "prereq.h"

#include <any/list.h>

struct test_node
{
    aint_t val;
    alist_node_t node;
};

TEST_CASE("list")
{
    alist_t l;
    alist_init(&l);
    REQUIRE(alist_is_end(&l, alist_head(&l)));
    REQUIRE(alist_is_end(&l, alist_back(&l)));
    enum { NUM_TEST_NODES = 5 };
    test_node nodes[NUM_TEST_NODES];
    for (aint_t i = 0; i < NUM_TEST_NODES; ++i) {
        nodes[i].val = i;
    }

    SECTION("push_head")
    {
        for (aint_t i = NUM_TEST_NODES - 1; i >= 0; --i) {
            alist_push_head(&l, &nodes[i].node);
        }
    }

    SECTION("push_back")
    {
        for (aint_t i = 0; i < NUM_TEST_NODES; ++i) {
            alist_push_back(&l, &nodes[i].node);
        }
    }

    {
        aint_t i;
        alist_node_t* itr;

        i = 0;
        itr = alist_head(&l);
        while (i < NUM_TEST_NODES) {
            REQUIRE(ALIST_NODE_CAST(test_node, itr)->val == i);
            REQUIRE(!alist_is_end(&l, itr));
            ++i; itr = itr->next;
        }
        REQUIRE(alist_is_end(&l, itr));

        i = NUM_TEST_NODES - 1;
        itr = alist_back(&l);
        while (i >= 0) {
            REQUIRE(ALIST_NODE_CAST(test_node, itr)->val == i);
            REQUIRE(!alist_is_end(&l, itr));
            --i; itr = itr->prev;
        }
        REQUIRE(alist_is_end(&l, itr));
    }
}