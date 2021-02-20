#!/usr/bin/env python2

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
        self.pip_install("wheel")
        self.install("locales")
        self.pip_install("setuptools --upgrade")
        self.install("git")

    def debian_compat(self):
        self.install("libatomic1")
        self.install("libgomp1")
        self.run("%s/bin/getgcc" % READIES)

    def redhat_compat(self):
        self.install("redhat-lsb-core")
        self.install("libatomic")
        self.install("libgomp")
        
        self.run("%s/bin/getgcc --modern" % READIES)

        # fix setuptools
        self.pip_install("-IU --force-reinstall setuptools")

    def fedora(self):
        self.install("libatomic")
        self.install("libgomp1")
        self.run("%s/bin/getgcc" % READIES)


    def common_last(self):
        self.run("{PYTHON} {READIES}/bin/getcmake".format(PYTHON=self.python, READIES=READIES))
        self.run("{PYTHON} {READIES}/bin/getrmpytools".format(PYTHON=self.python, READIES=READIES))
        self.install("lcov")
        self.pip_install("pudb awscli")
        self.pip_install("-r %s/tests/requirements.txt" % ROOT)

#----------------------------------------------------------------------------------------------

parser = argparse.ArgumentParser(description='Set up system for build.')
parser.add_argument('-n', '--nop', action="store_true", help='no operation')
args = parser.parse_args()

RedisGraphSetup(nop = args.nop).setup()
