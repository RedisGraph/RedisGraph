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
        self.pip_install("wheel")
        self.install("locales")
        self.pip_install("setuptools --upgrade")
        self.install("git")

    def debian_compat(self):
        self.install("python3-devel")
        self.install("libatomic1")
        self.install("libgomp1")
        self.run("%s/bin/getgcc" % READIES)

    def redhat_compat(self):
        self.group_install("'Development Tools'")
        self.install("libatomic")
        self.install("libgomp")
        self.install("redhat-lsb-core")
        self.run("%s/bin/getgcc --modern" % READIES)

        if not self.dist == "amzn":
            self.install("epel-release")
            self.install("python3-devel libaec-devel")
        else:
            self.run("amazon-linux-extras install epel")
            self.install("python3-devel")

    def common_last(self):
        self.install("lcov")
        self.run("python3 %s/bin/getrmpytools" % READIES) 
        self.pip_install("-r tests/flow/requirements.txt")
        self.run("python3 %s/bin/getcmake" % READIES)
        self.pip_install("pudb awscli")

#----------------------------------------------------------------------------------------------

parser = argparse.ArgumentParser(description='Set up system for build.')
parser.add_argument('-n', '--nop', action="store_true", help='no operation')
args = parser.parse_args()

RedisGraphSetup(nop = args.nop).setup()
