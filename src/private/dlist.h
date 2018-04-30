// Copyright (c) 2017-2018 Nguyen Viet Giang. All rights reserved.
#ifndef _AVM_DLIST_H_
#define _AVM_DLIST_H_

#include <avm/prereq.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Intrusive linked list node.
typedef struct anode_s {
    struct anode_s *next;
    struct anode_s *prev;
} anode_t;

/// Initialize `n` as a new list.
AINLINE void
alist_init(
    anode_t *n)
{
    n->next = n;
    n->prev = n;
}

/// Add `n` between `next` and `prev`.
AINLINE void
alist_insert(
    anode_t *n, anode_t *prev, anode_t *next)
{
    n->next = next;
    n->prev = prev;
    next->prev = n;
    prev->next = n;
}

/// Add `n` to head of the list `l`.
AINLINE void
alist_push_head(
    anode_t *l, anode_t *n)
{
    alist_insert(n, l, l->next);
}

/// Add `n` to back of the list `l`.
AINLINE void
alist_push_back(
    anode_t *l, anode_t *n)
{
    alist_insert(n, l->prev, l);
}

/// Remove `n` links.
AINLINE void
anode_unlink(
    anode_t *n)
{
    n->next->prev = n->prev;
    n->prev->next = n->next;
}

/// Returns the head node.
AINLINE anode_t*
alist_head(
    anode_t *l)
{
    return l->next;
}

/// Returns the last node.
AINLINE anode_t*
alist_back(
    anode_t *l)
{
    return l->prev;
}

/// Check whether `n` is a valid node.
AINLINE abool
alist_not_end(
    anode_t *l, anode_t *n)
{
    return l != n;
}

/// Check whether `n` is a valid node.
AINLINE abool
alist_end(
    anode_t *l, anode_t *n)
{
    return l == n;
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !_AVM_DLIST_H_