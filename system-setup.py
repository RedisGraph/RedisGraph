#!/usr/bin/env python3

import sys
import os
import argparse

ROOT = HERE = os.path.abspath(os.path.dirname(__file__))
READIES = os.path.join(ROOT, "deps/readies")
sys.path.insert(0, READIES)
import paella

#----------------------------------------------------------------------------------------------

class RedisGraphSetup(paella.Setup):
    def __init__(self, nop=False):
        paella.Setup.__init__(self, nop)

    def common_first(self):
        self.install_downloaders()
        self.pip_install("wheel virtualenv")
        self.pip_install("setuptools --upgrade")

        self.install("git automake peg libtool autoconf valgrind astyle")

    def debian_compat(self):
        self.run("%s/bin/getgcc" % READIES)
        # self.install("python-psutil")

    def redhat_compat(self):
        self.install("redhat-lsb-core")
        self.run("%s/bin/getgcc --modern" % READIES)
        # self.pip_install("psutil")

    def fedora(self):
        self.run("%s/bin/getgcc" % READIES)
        # self.install("python-psutil")

    def macos(self):
        self.install_gnu_utils()
        self.install("redis")

    def common_last(self):
        self.run("%s/bin/getcmake" % READIES)
        self.run("{PYTHON} {READIES}/bin/getrmpytools".format(PYTHON=self.python, READIES=READIES))

        self.pip_install("-r tests/requirements.txt")

#----------------------------------------------------------------------------------------------

parser = argparse.ArgumentParser(description='Set up system for build.')
parser.add_argument('-n', '--nop', action="store_true", help='no operation')
args = parser.parse_args()

RedisGraphSetup(nop=args.nop).setup()
