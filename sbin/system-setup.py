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
    def __init__(self, args):
        paella.Setup.__init__(self, args.nop)
        self.no_rmpytools = args.no_rmpytools

    def common_first(self):
        self.install_downloaders()

        self.run(f"{READIES}/bin/enable-utf8", sudo=self.os != 'macos')
        self.install("git automake libtool autoconf")

    def debian_compat(self):
        self.install("locales")
        self.run(f"{READIES}/bin/getgcc --modern")
        self.install("peg")
        if self.platform.is_arm():
            self.install("python3-dev")
        self.run(f"{READIES}/bin/getjava") # for grammarinator/ANTLR
        self.pip_install("-r tests/fuzz/requirements.txt --use-pep517")

    def redhat_compat(self):
        self.install("redhat-lsb-core")
        self.install("which")
        if not self.platform.is_arm():
            self.install_linux_gnu_tar()
        if self.osnick == 'ol8':
            self.install("which") # for automake
        self.run(f"{READIES}/bin/getepel", sudo=True)
        self.run(f"{READIES}/bin/getgcc --modern")
        self.install("m4 libgomp")
        self.install_peg()

    def fedora(self):
        self.run(f"{READIES}/bin/getgcc")
        self.install_peg()

    def macos(self):
        self.install_gnu_utils()
        # self.run(f"{READIES}/bin/getgcc --modern")
        self.run("brew install libomp")
        self.install("redis")
        self.install_peg()
        self.pip_install("-r tests/fuzz/requirements.txt --use-pep517")

    def alpine(self):
        self.install("automake make autoconf libtool m4")
        self.run(f"{READIES}/bin/getgcc")
        self.install_peg()

    def linux_last(self):
        self.install("valgrind")

    def common_last(self):
        self.run(f"{READIES}/bin/getaws")
        self.install("astyle", _try=True) # fails for centos7
        self.run(f"{self.python} {READIES}/bin/getcmake --usr", sudo=self.os != 'macos')
        if self.dist != "arch":
            self.install("lcov")
        else:
            self.install("lcov-git", aur=True)

        if not self.no_rmpytools:
            self.run(f"{self.python} {READIES}/bin/getrmpytools --reinstall")
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
parser.add_argument('-n', '--nop', action="store_true", help='No operation')
parser.add_argument('--no-rmpytools', action="store_true", help='Do not install Python tools')
args = parser.parse_args()

RedisGraphSetup(args).setup()
