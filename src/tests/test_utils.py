"""Unit Test for utilities"""
import os
from hypothesis.strategies import integers
from hypothesis import given
from qcheck.cffi_helpers import load_ffi

PATH = os.path.dirname(__file__)
m = load_ffi(os.path.join(PATH, 'test_utils.c'), None)
ffi = m.ffi
lib = m.lib

@given(integers(min_value=1, max_value=0x80000000))
def test_powof2x_ceil(v):
    """Test round-up power of 2"""
    p = 0
    for i in range(0, 32):
        p = pow(2, i)
        if p >= v:
            break
    assert p == lib.apowof2_ceil(v)
