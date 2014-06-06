#!/usr/bin/env python

# run tests with: python -m unittest test_model
#             or  python -m unittest discover

import os
import sys
import unittest
import fityk


class TestCopy(unittest.TestCase):
    def setUp(self):
        self.ftk = fityk.Fityk()
        self.ftk.set_option_as_number("verbosity", -1)

    def test_copy_func(self):
        self.ftk.execute("%f = Gaussian(~10, ~0, 0.5)")
        self.ftk.execute("%g = copy(%f)")
        self.assertEqual(self.ftk.get_parameter_count(), 4)
        self.ftk.execute("%g.center = ~2.3")
        self.assertEqual(self.ftk.get_parameter_count(), 4)
        self.ftk.execute("F = %f + %g")
        self.ftk.execute("@+ = 0")
        self.ftk.execute("@1.F = copy(@0.F)")
        self.assertEqual(self.ftk.get_parameter_count(), 8)
        self.ftk.execute("@+ = 0")
        self.ftk.execute("@2.F = @0.F")
        self.assertEqual(self.ftk.get_parameter_count(), 8)
        self.ftk.execute("%g.center = ~2.4")
        d0_funcs = self.ftk.get_components(0)
        d1_funcs = self.ftk.get_components(1)
        d2_funcs = self.ftk.get_components(2)
        self.assertEqual(d0_funcs[0].get_param_value('center'), 0)
        self.assertEqual(d0_funcs[1].get_param_value('center'), 2.4)
        self.assertEqual(d1_funcs[0].get_param_value('center'), 0)
        self.assertEqual(d1_funcs[1].get_param_value('center'), 2.3)
        self.assertEqual(d2_funcs[0].get_param_value('center'), 0)
        self.assertEqual(d2_funcs[1].get_param_value('center'), 2.4)

    def test_copy_var(self):
        self.ftk.execute("%f = Gaussian(~10, ~0, 0.5)")
        self.ftk.execute("$v = copy(%f.height)")
        v = self.ftk.get_variable("v")
        self.assertEqual(v.value(), 10)

class TestBounds(unittest.TestCase):
    def setUp(self):
        self.ftk = fityk.Fityk()
        self.ftk.set_option_as_number("verbosity", -1)
        self.ftk.execute("$a = ~3.14 [-1:4.5]")

    def test_basic(self):
        a = self.ftk.get_variable("a")
        self.assertEqual(a.value(), 3.14)
        self.assertEqual(a.domain.lo, -1)
        self.assertEqual(a.domain.hi, 4.5)
        self.assertFalse(a.domain.lo_inf())
        self.assertFalse(a.domain.hi_inf())

    def test_change_value(self):
        a = self.ftk.get_variable("a")
        self.ftk.execute("$a = ~3.15")
        self.assertEqual(a.domain.lo, -1)
        self.assertEqual(a.domain.hi, 4.5)
        self.ftk.execute("$a = ~3.15 [:]") # invalidates a, i.e.
        a2 = self.ftk.get_variable("a")    # a2 != a
        self.assertTrue(a2.domain.lo_inf())
        self.assertTrue(a2.domain.hi_inf())

    def test_assign_var(self):
        self.ftk.execute("$b = $a") # b is compound variable
        b = self.ftk.get_variable("b")
        self.assertEqual(b.value(), 3.14)
        self.assertTrue(b.domain.lo_inf())
        self.assertTrue(b.domain.hi_inf())
        self.ftk.execute("$c = ~0.1 [0.0:0.2]")
        self.ftk.execute("$c = $a")
        c = self.ftk.get_variable("c")
        self.assertEqual(c.value(), 3.14)
        self.assertEqual(c.domain.lo, 0.0)
        self.assertEqual(c.domain.hi, 0.2)

    def test_assign_value(self):
        self.ftk.execute("$b = ~{$a}")
        b = self.ftk.get_variable("b")
        self.assertEqual(b.value(), 3.14)
        self.assertTrue(b.domain.lo_inf())
        self.assertTrue(b.domain.hi_inf())

    def test_copy_var(self):
        self.ftk.execute("$b = copy($a)")
        b = self.ftk.get_variable("b")
        self.assertEqual(b.value(), 3.14)
        self.assertEqual(b.domain.lo, -1)
        self.assertEqual(b.domain.hi, 4.5)

if __name__ == '__main__':
    unittest.main()
