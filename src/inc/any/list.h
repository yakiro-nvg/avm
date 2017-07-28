/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

// Intrusive linked list node.
typedef struct alist_node_s {
    struct alist_node_s* next;
    struct alist_node_s* prev;
} alist_node_t;

// Intrusive linked list.
typedef struct {
    alist_node_t root;
} alist_t;

/// Initialize as a new list.
AINLINE void alist_init(alist_t* self)
{
    self->root.next = &self->root;
    self->root.prev = &self->root;
}

/// Add node between next and previous.
AINLINE void alist_node_insert(
    alist_node_t* self, alist_node_t* prev, alist_node_t* next)
{
    self->next = next;
    self->prev = prev;
    next->prev = self;
    prev->next = self;
}

/// Add node to head of list.
AINLINE void alist_push_head(alist_t* self, alist_node_t* node)
{
    alist_node_insert(node, &self->root, self->root.next);
}

/// Add node to back of list.
AINLINE void alist_push_back(alist_t* self, alist_node_t* node)
{
    alist_node_insert(node, self->root.prev, &self->root);
}

/// Remove node from list.
AINLINE void alist_node_erase(alist_node_t* self)
{
    self->next->prev = self->prev;
    self->prev->next = self->next;
#ifdef ANY_DEBUG
    self->next = NULL;
    self->prev = NULL;
#endif
}

/// Return the fist node.
AINLINE alist_node_t* alist_head(alist_t* self)
{
    return self->root.next;
}

/// Return the last node.
AINLINE alist_node_t* alist_back(alist_t* self)
{
    return self->root.prev;
}

/// Check whether node is end.
AINLINE int32_t alist_is_end(alist_t* self, alist_node_t* node)
{
    return &self->root == node;
}

#define ALIST_NODE_CAST(T, n) ((T*)(((uint8_t*)n) - offsetof(T, node)))

#ifdef __cplusplus
} // extern "C"
#endif