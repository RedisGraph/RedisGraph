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

        self.run("%s/bin/enable-utf8" % READIES, sudo=self.os != 'macos')
        self.install("git automake libtool autoconf")

    def debian_compat(self):
        self.install("locales")
        if self.platform.is_arm():
            self.run("%s/bin/getgcc --modern" % READIES)
        else:
            self.run("%s/bin/getgcc" % READIES)
        self.install("peg")

    def redhat_compat(self):
        self.install("redhat-lsb-core")
        if self.osnick == 'ol8':
            self.install("which") # for automake
        self.run("%s/bin/getepel" % READIES, sudo=True)
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

    def alpine(self):
        self.install("automake make autoconf libtool m4")
        self.run("%s/bin/getgcc" % READIES)
        self.install_peg()

    def linux_last(self):
        self.install("valgrind")

    def common_last(self):
        self.install("astyle", _try=True) # fails for centos7
        self.run("{PYTHON} {READIES}/bin/getcmake --usr".format(PYTHON=self.python, READIES=READIES),
                 sudo=self.os != 'macos')
        if self.dist != "arch":
            self.install("lcov")
        else:
            self.install("lcov-git", aur=True)

        self.run("{PYTHON} {READIES}/bin/getrmpytools --reinstall".format(PYTHON=self.python, READIES=READIES))
        self.pip_install("-r tests/requirements.txt")

    def install_peg(self):
        self.run(r"""
            cd /tmp
            build_dir=$(mktemp -d)
            cd $build_dir
            wget -q -O peg.tar.gz https://github.com/gpakosz/peg/archive/0.1.18.tar.gz
            tar xzf peg.tar.gz
            cd peg-0.1.18
            make
            $(command -v sudo) make install MANDIR=.
            cd /tmp
            rm -rf $build_dir
            """)

#----------------------------------------------------------------------------------------------

parser = argparse.ArgumentParser(description='Set up system for build.')
parser.add_argument('-n', '--nop', action="store_true", help='no operation')
args = parser.parse_args()

RedisGraphSetup(nop=args.nop).setup()
