ROOT=.

MK_ALL_TARGETS=bindirs deps build package

ifeq ($(wildcard $(ROOT)/deps/readies/*),)
___:=$(shell git submodule update --init --recursive &> /dev/null)
endif

include $(ROOT)/deps/readies/mk/main

MK_CUSTOM_CLEAN=1
BINDIR=$(BINROOT)

.PHONY: all parser clean package upload-artifacts upload-release docker \
	builddocs localdocs deploydocs test benchmark fuzz help

define HELPTEXT
make all            # Build everything
  DEBUG=1             # Build for debugging
  COV=1               # Build for coverage analysis (implies DEBUG=1)
  SLOW=1              # Disable parallel build
  STATIC_OMP=1        # Link OpenMP statically
  VARIANT=name        # Add `name` to build products directory
make clean          # Clean build products
  ALL=1               # Completely remove build products
  DEPS=1              # Also clean dependant modules
  AUTOGEN=1           # Remove autogen-generated files
make pack|package   # Build RAMP packages
make run            # Run redis-server with RedisGraph
  GDB=1               # Run with debugger

make test         # Run tests
  HELP=1            # Show testing options
  LIST=1            # List all tests, do not execute
  UNIT=1            # Run unit tests
  FLOW=1            # Run flow tests (Python)
  TCK=1             # Run TCK framework tests
  COV=1             # Perform coverage analysis
  SLOW=1            # Do not run in parallel
  PARALLEL=n        # Set testing parallelism
  GDB=1             # RLTest interactive debugging
  TEST=test         # Run specific test
  TESTFILE=file     # Run tests listed in file
  FAILFILE=file     # Write failed tests to file

make memcheck    # Run tests with Valgrind
make benchmark   # Run benchmarks
make fuzz        # Run fuzz tester

make coverage     # Perform coverage analysis (build & test)
make cov-upload   # Upload coverage data to codecov.io

make upload-artifacts   # copy snapshot packages to S3
  OSNICK=nick             # copy snapshots for specific OSNICK
make upload-release     # copy release packages to S3

common options for upload operations:
  STAGING=1   # copy to staging lab area (for validation)
  FORCE=1     # allow operation outside CI environment
  VERBOSE=1   # show more details
  NOP=1       # do not copy, just print commands

make docker     # build for specified platform
  OSNICK=nick     # platform to build for (default: host platform)
  TEST=1          # run tests after build
  PACK=1          # create package
  ARTIFACTS=1     # copy artifacts to host

endef

include $(MK)/defs

all:
	@$(MAKE) -C src all -j $(NPROC)

include $(MK)/rules

build:
	@$(MAKE) -C src build -j $(NPROC)

clean:
	@$(MAKE) -C src $@

clean-parser:
	$(MAKE) -C build/libcypher-parser clean

clean-graphblas:
	$(MAKE) -C build/GraphBLAS clean

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

coverage:
	@$(MAKE) -C src coverage

fuzz:
	@$(MAKE) -C src fuzz
