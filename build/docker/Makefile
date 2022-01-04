
ROOT=../..
MK.pyver:=3
include $(ROOT)/deps/readies/mk/main

REDIS_VERSION=6.2.6
REPO=redisgraph

OSNICK.official=bullseye

INT_BRANCHES=2.8 2.6 2.4 1.2 1.0
LATEST_BRANCH=2.4
PREVIEW_BRANCH=2.8

ART_DIR=$(ROOT)/bin/artifacts

include $(MK)/module.docker.rules
