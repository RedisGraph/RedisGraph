
ROOT=../..

include $(ROOT)/deps/readies/mk/main

REPO=redisgraph

REDIS_VERSION=6.2.10
OSNICK.official=bionic

INT_BRANCHES=2.10 2.8 2.6 2.4 2.2 1.2 1.0
LATEST_BRANCH=2.10
PREVIEW_BRANCH=2.10

ART_DIR=$(ROOT)/bin/artifacts

XENIAL_MODERN_OMP ?= 0

DOCKER_VARS += XENIAL_MODERN_OMP

include $(MK)/module.docker.rules
