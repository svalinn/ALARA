import sys
import os
sys.path.insert(0, '../')
from nose.tools import assert_equal

from elelib_to_nuclib import elelib_to_nuclib

def test_elelib_to_nuclib():
    thisdir = os.path.dirname(__file__)
    inp = os.path.join(thisdir, "test_inp")
    out = os.path.join(thisdir,"test_out")
    elelib_to_nuclib(inp, out)
    with open(out, 'r') as f:
        actual = "".join(f.readlines())
    expected = (
    'he      0.400260E+01   2      0.178700E-03   2\n'
    '                                                 3      0.100000E-03\n'
    '                                                 4      0.100000E+03\n' 
    'he:3   3.01603E+00   2   1.34654E-04 1\n' 
    '    3 100\n'
    'he:4   4.00260E+00   2   1.78700E-04 1\n'
    '    4 100\n'
    'na      0.229898E+02  11      0.970000E+00   1\n'
    '                                                23      0.100000E+03\n'
    'na:23   2.29898E+01  11   9.69999E-01 1\n'
    '    23 100\n')
    assert_equal(actual, expected)
    os.remove(out)
