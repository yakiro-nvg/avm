import os
from qcheck.cffi_helpers import load_ffi

# generate uncovered files
def test_gcov():
    PATH = os.path.dirname(__file__)
    m = load_ffi(os.path.join(PATH, 'test_gcov.c'), [
        'anode_t', 'astack_t'
    ])
    lib = m.lib
    lib.flush_coverage()