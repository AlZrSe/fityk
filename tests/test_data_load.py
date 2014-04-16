#!/usr/bin/env python

# run tests with: python -m unittest test_data_load
#             or  python -m unittest discover

import os
import sys
import random
import tempfile
import gzip
import unittest
import fityk

#locale.setlocale(locale.LC_NUMERIC, 'C')

class FileLoadBase(unittest.TestCase):
    def setUp(self):
        self.ftk = fityk.Fityk()
        self.ftk.set_option_as_number("verbosity", -1)
        f = tempfile.NamedTemporaryFile(delete=False)
        self.data = [(random.uniform(-100, 100), random.gauss(10, 20))
                     for _ in range(30)]
        for d in self.data:
            f.write(self.line_format(d))
        f.close()
        self.data.sort()
        self.filename = f.name

    def tearDown(self):
        #print "remove " + self.filename
        os.unlink(self.filename)

    def compare(self, out, ndigits):
        self.assertEqual([i.x for i in out],
                         [round(i[0], ndigits) for i in self.data])
        self.assertEqual([i.y for i in out],
                         [round(i[1], ndigits) for i in self.data])

class TestText(FileLoadBase):
    def line_format(self, t):
        return " %.7f %.7f\n" % t

    def test_load(self):
        self.ftk.execute("@0 < '%s'" % self.filename)
        self.compare(self.ftk.get_data(), 7)
    def test_load_strict(self):
        self.ftk.execute("@0 < '%s' text strict" % self.filename)
        self.compare(self.ftk.get_data(), 7)
    def test_load_decimal_comma(self):
        # this option only replaces commas with dots, so it's still possible
        # to read file with decimal point '.'
        self.ftk.execute("@0 < '%s' text decimal_comma" % self.filename)
        self.compare(self.ftk.get_data(), 7)

class TestTextComma(FileLoadBase):
    def line_format(self, t):
        return (" %.7f %.7f\n" % t).replace('.', ',')

    def test_load(self):
        self.ftk.execute("@0 < '%s' text decimal_comma" % self.filename)
        self.compare(self.ftk.get_data(), 7)
    def test_load_strict(self):
        self.ftk.execute("@0 < '%s' text decimal_comma strict" % self.filename)
        self.compare(self.ftk.get_data(), 7)
    def test_load_decimal_point(self): # should fail
        self.ftk.execute("@0 < '%s'" % self.filename)
        self.assertNotEqual([i.x for i in self.ftk.get_data()],
                            [round(i[0], 7) for i in self.data])


class TestSimpleScript(unittest.TestCase):
    def setUp(self):
        self.ftk = fityk.Fityk()
        self.ftk.set_option_as_number("verbosity", -1)
        f = tempfile.NamedTemporaryFile(delete=False)
        f.write("M=1\n X[0]=1.1, Y[0]=-5, S[0]=0.8")
        f.close()
        self.filename = f.name

    def tearDown(self):
        os.unlink(self.filename)

    def test_simple_script(self):
        self.ftk.get_ui_api().exec_fityk_script(self.filename)
        data = self.ftk.get_data()
        self.assertEqual(len(data), 1)
        self.assertEqual(data[0].x, 1.1)
        self.assertEqual(data[0].y, -5)
        self.assertEqual(data[0].sigma, 0.8)

class TestGzippedScript(unittest.TestCase):
    def setUp(self):
        self.ftk = fityk.Fityk()
        self.ftk.set_option_as_number("verbosity", -1)
        f = tempfile.NamedTemporaryFile(suffix='.gz', delete=False)
        self.filename = f.name
        gf = gzip.GzipFile(fileobj=f)
        gf.write("M=1\n X[0]=1.1, Y[0]=-5, S[0]=0.8")
        gf.close()

    def tearDown(self):
        os.unlink(self.filename)

    def test_gzipped_script(self):
        self.ftk.get_ui_api().exec_fityk_script(self.filename)
        data = self.ftk.get_data()
        self.assertEqual(len(data), 1)
        self.assertEqual(data[0].x, 1.1)
        self.assertEqual(data[0].y, -5)
        self.assertEqual(data[0].sigma, 0.8)


if __name__ == '__main__':
    unittest.main()

