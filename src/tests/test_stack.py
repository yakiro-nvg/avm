import os
from qcheck.cffi_helpers import load_ffi
from hypothesis.strategies import *
from hypothesis.stateful import *

PATH = os.path.dirname(__file__)

class DynamicStackTest(RuleBasedStateMachine):
    m = load_ffi(os.path.join(PATH, 'test_stack.c'), [
        'anode_t', 'avalue_t', 'avalue_stack_t'
    ])
    ffi = m.ffi
    lib = m.lib
    values = Bundle('values')

    def __init__(self):
        super(DynamicStackTest, self).__init__()
        self.values = []
        self.mstack = []
        self.vstack = self.ffi.new('avalue_stack_t*')
        assert(self.lib.stack_init(self.vstack, 2) == self.lib.AR_SUCCESS)
        assert(self.vstack.cap == 2)

    @rule(target=values, v=floats(allow_nan=False))
    def v(self, v):
        return v

    @rule(n=integers(min_value=0, max_value=10000))
    def grow(self, n):
        cap = self.vstack.cap
        r = self.lib.avalue_stack_grow(self.vstack, n)
        if r == self.lib.AR_SUCCESS:
            assert(self.vstack.sp + n <= self.vstack.cap)
        else:
            assert(cap == self.vstack.cap)

    @rule(n=integers(min_value=0, max_value=10000))
    def reserve(self, n):
        cap = self.vstack.cap
        r = self.lib.avalue_stack_reserve(self.vstack, n)
        if r == self.lib.AR_SUCCESS:
            assert(self.vstack.sp + n <= self.vstack.cap)
        else:
            assert(cap == self.vstack.cap)

    @rule(v=values)
    def push(self, v):
        x = self.ffi.new('avalue_t*'); x[0] = v
        r = self.lib.avalue_stack_push(self.vstack, x)
        if r == self.lib.AR_SUCCESS:
            self.values.append(x)
            self.mstack.append(v)

    @rule(v=values, n=integers(min_value=0, max_value=10000))
    def fill(self, v, n):
        x = self.ffi.new('avalue_t*'); x[0] = v
        r = self.lib.avalue_stack_fill(self.vstack, x, n)
        if r == self.lib.AR_SUCCESS:
            self.values.append(x)
            for i in range(n):
                self.mstack.append(v)

    @invariant()
    def same_contents(self):
        assert(len(self.mstack) == self.vstack.sp)
        for i in range(len(self.mstack)):
            assert(self.vstack.items[i] == self.mstack[i])

    def teardown(self):
        self.lib.avalue_stack_cleanup(self.vstack)
        self.lib.flush_coverage()

TestTrees = DynamicStackTest.TestCase