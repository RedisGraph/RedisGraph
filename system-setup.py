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

        self.install("git automake libtool autoconf valgrind astyle")

    def debian_compat(self):
        self.install("locales")
        self.run("%s/bin/getgcc" % READIES)
        self.install("peg")

    def redhat_compat(self):
        self.install("redhat-lsb-core")
        self.run("%s/bin/getgcc --modern" % READIES)
        self.install("m4 libgomp")
        self.install_peg()

    def fedora(self):
        self.run("%s/bin/getgcc" % READIES)
        self.install_peg()

    def macos(self):
        self.install_gnu_utils()
        self.install("redis")
        self.install_peg()

    def common_last(self):
        self.run("%s/bin/getcmake" % READIES)
        self.run("{PYTHON} {READIES}/bin/getrmpytools".format(PYTHON=self.python, READIES=READIES))

        self.pip_install("-r tests/requirements.txt")

    def install_peg(self):
        self.run(r"""
            wget https://www.piumarta.com/software/peg/peg-0.1.18.tar.gz
            tar xzf peg-0.1.18.tar.gz
            cd peg-0.1.18
            make
            make install
            """)

#----------------------------------------------------------------------------------------------

parser = argparse.ArgumentParser(description='Set up system for build.')
parser.add_argument('-n', '--nop', action="store_true", help='no operation')
args = parser.parse_args()

RedisGraphSetup(nop=args.nop).setup()
