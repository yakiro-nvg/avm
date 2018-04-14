#include <value_stack.c>

void*
stack_realloc(
    aalloc_t *a, void *old, u32 sz)
{
    return sz <= 10000 ? realloc(old, sz) : NULL;
}

aresult_t
stack_init(
    avalue_stack_t *s, u32 capacity)
{
    static aalloc_t a = { &stack_realloc };
    return avalue_stack_init(s, &a, capacity);
}