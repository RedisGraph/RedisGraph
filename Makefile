ROOT=.

MK_ALL_TARGETS=bindirs deps build package

ifeq ($(wildcard $(ROOT)/deps/readies/*),)
___:=$(shell git submodule update --init --recursive &> /dev/null)
endif

MK.pyver:=3
include $(ROOT)/deps/readies/mk/main

MK_CUSTOM_CLEAN=1
BINDIR=$(BINROOT)

.PHONY: all parser clean package docker upload-artifacts upload-release docker_push docker_alpine \
	builddocs localdocs deploydocs test benchmark test_valgrind fuzz help

define HELPTEXT
make all                # Build everything
  DEBUG=1                 # Build for debugging
  COV=1                   # Build for coverage analysis (implies DEBUG=1)
  STATIC_OMP=1            # Link OpenMP statically
make clean              # Clean build artifacts
  ALL=1                   # Completely remove
make run                # run redis-server with RedisGraph
make test               # Run tests
  LIST=1                   # List all tests, do not execute
  UNIT=1                   # Run unit tests
  FLOW=1                   # Run flow tests (Python)
  TCK=1                    # Run TCK framework tests
  COV=1                    # Perform coverage analysis
  SLOW=1                   # Do not run in parallel
  TEST=test                # Run specific test
  TESTFILE=file            # Run tests listed in file
  FAILFILE=file            # Write failed tests to file
make memcheck           # Run tests with Valgrind
make benchmark          # Run benchmarks
make fuzz               # Run fuzz tester

make package            # Build RAMP packages
make cov-upload         # Upload coverage data to codecov.io

make upload-artifacts   # copy snapshot packages to S3
  OSNICK=nick             # copy snapshots for specific OSNICK
make upload-release     # copy release packages to S3

common options for upload operations:
  STAGING=1             # copy to staging lab area (for validation)
  FORCE=1               # allow operation outside CI environment
  VERBOSE=1             # show more details
  NOP=1                 # do not copy, just print commands

endef

include $(MK)/defs

all:
	@$(MAKE) -C src all

include $(MK)/rules

clean:
	@$(MAKE) -C src $@

clean-parser:
	$(MAKE) -C deps/libcypher-parser distclean

clean-graphblas:
	$(MAKE) -C deps/GraphBLAS clean

pack package: all
	@$(MAKE) -C src package

run:
	@$(MAKE) -C src run

upload-release:
	@RELEASE=1 ./sbin/upload-artifacts

upload-artifacts:
	@SNAPSHOT=1 ./sbin/upload-artifacts

docker:
	@$(MAKE) -C build/docker

docker_alpine:
	@$(MAKE) -C build/docker OSNICK=alpine3

docker_push: docker
	@docker push redislabs/redisgraph:latest

builddocs:
	@mkdocs build

localdocs: builddocs
	@mkdocs serve

deploydocs: builddocs
	@mkdocs gh-deploy

test:
	@$(MAKE) -C src test

benchmark:
	@$(MAKE) -C src benchmark

memcheck:
	@$(MAKE) -C src memcheck

cov-upload:
	@$(MAKE) -C src cov-upload

fuzz:
	@$(MAKE) -C src fuzz

