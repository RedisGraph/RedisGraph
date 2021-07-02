#!/usr/bin/env python3

import sys
import os
import argparse

HERE = os.path.dirname(__file__)
ROOT = os.path.abspath(os.path.join(HERE, ".."))
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

        self.run("%s/bin/enable-utf8" % READIES)
        self.install("git automake libtool autoconf")

    def debian_compat(self):
        self.install("locales")
        self.run("%s/bin/getgcc" % READIES)
        self.install("peg")

    def redhat_compat(self):
        self.install("redhat-lsb-core")
        self.run("%s/bin/getepel" % READIES)
        self.run("%s/bin/getgcc --modern" % READIES)
        self.install("m4 libgomp")
        self.install_peg()

    def fedora(self):
        self.run("%s/bin/getgcc" % READIES)
        self.install_peg()

    def macos(self):
        self.install_gnu_utils()
        self.run("%s/bin/getgcc --modern" % READIES)
        self.install("redis")
        self.install_peg()

    def linux_last(self):
        self.install("valgrind")

    def common_last(self):
        self.install("astyle", _try=True) # fails for centos7
        self.run("%s/bin/getcmake" % READIES)
        self.run("{PYTHON} {READIES}/bin/getrmpytools".format(PYTHON=self.python, READIES=READIES))

        self.pip_install("-r tests/requirements.txt")

    def install_peg(self):
        self.run(r"""
            cd /tmp
            build_dir=$(mktemp -d)
            cd $build_dir
            wget https://www.piumarta.com/software/peg/peg-0.1.18.tar.gz
            tar xzf peg-0.1.18.tar.gz
            cd peg-0.1.18
            make
            make install
            cd /tmp
            rm -rf $build_dir
            """)

#----------------------------------------------------------------------------------------------

parser = argparse.ArgumentParser(description='Set up system for build.')
parser.add_argument('-n', '--nop', action="store_true", help='no operation')
args = parser.parse_args()

RedisGraphSetup(nop=args.nop).setup()
