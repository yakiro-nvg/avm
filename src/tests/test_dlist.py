"""Unit Test for double linked list"""
import os
from first import first
from hypothesis.stateful import rule, invariant, RuleBasedStateMachine, Bundle
from hypothesis.strategies import floats
from pyllist import dllist
from qcheck.cffi_helpers import load_ffi

PATH = os.path.dirname(__file__)

class DoublyLinkedListTest(RuleBasedStateMachine):
    """Stateful test for double linked list"""

    m = load_ffi(os.path.join(PATH, 'test_dlist.c'), [
        'anode_t', 'value_t'
    ])
    ffi = m.ffi
    lib = m.lib
    values = Bundle('values')

    def __init__(self):
        super(DoublyLinkedListTest, self).__init__()
        self.nodes = []
        self.mlist = dllist()
        self.dlist = self.ffi.new('anode_t*')
        self.lib.alist_init(self.dlist)

    @rule(target=values, v=floats(allow_nan=False))
    def v(self, v):
        """Generate values"""
        return v

    @rule(v=values)
    def push_head(self, v):
        """Push value to head"""
        self.mlist.appendleft(v)
        n = self.ffi.new('value_t*')
        n.value = v
        self.lib.alist_push_head(
            self.dlist,
            self.ffi.cast('anode_t*', n))
        self.nodes.append(n)
        assert self.lib.alist_head(self.dlist) == n

    @rule(v=values)
    def push_back(self, v):
        """Push value to back"""
        self.mlist.appendright(v)
        n = self.ffi.new('value_t*')
        n.value = v
        self.lib.alist_push_back(
            self.dlist,
            self.ffi.cast('anode_t*', n))
        self.nodes.append(n)
        assert self.lib.alist_back(self.dlist) == n

    @rule(v=values)
    def unlink(self, v):
        """Remove node links"""
        mi = first(self.mlist.iternodes(), key=lambda x: x.value == v)
        di = self.lib.alist_head(self.dlist)
        while self.lib.alist_not_end(self.dlist, di):
            if self.ffi.cast('value_t*', di).value == v:
                break
            di = di.next
        if self.lib.alist_not_end(self.dlist, di):
            assert mi != None
            self.mlist.remove(mi)
            self.lib.anode_unlink(di)
            self.nodes.remove(di)
        else:
            assert mi is None

    @invariant()
    def same_contents(self):
        """Sanity check"""
        mi = self.mlist.first
        di = self.lib.alist_head(self.dlist)
        while self.lib.alist_not_end(self.dlist, di):
            assert mi.value == self.ffi.cast('value_t*', di).value
            mi = mi.next
            di = di.next
        assert mi is None

    def teardown(self):
        self.lib.flush_coverage()

TestTrees = DoublyLinkedListTest.TestCase
