
ifeq ($(VG),docker)
override VG:=1
VG_DOCKER=1
endif

ifeq ($(VG),1)
export MEMCHECK=1
endif

ifneq ($(SAN),)
export MEMCHECK=1
endif

#----------------------------------------------------------------------------------------------

.NOTPARALLEL:

ROOT=.

MK.cmake=1
SRCDIR=.

include $(ROOT)/deps/readies/mk/main

MK_ALL_TARGETS=bindirs deps build

MK_CUSTOM_CLEAN=1

BINDIR=$(BINROOT)/src
export TARGET=$(BINROOT)/src/redisgraph.so

#----------------------------------------------------------------------------------------------

define HELPTEXT
make all            # Build everything
  DEBUG=1             # Build for debugging
  SLOW=1              # Disable parallel build
  STATIC_OMP=1        # Link OpenMP statically
  VARIANT=name        # Add `name` to build products directory
  GCC=1               # Build with GCC toolchain (default nor Linux)
  CLANG=1             # Build with CLang toolchain (default for macOS)
  COV=1               # Build for coverage analysis (implies DEBUG=1)
  VG=1|docker         # build for Valgrind
  SAN=type            # build with LLVM sanitizer (type=address|memory|leak|thread) 
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

make unit-tests   # Run unit tests
make flow-tests   # Run flow tests
make tck-tests    # Run TCK tests
make fuzz-tests   # Run fuzz tester
  TIMEOUT=secs      # Timeout in `secs`

make benchmark    # Run benchmarks
  REMOTE=1          # Run remotely

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

#----------------------------------------------------------------------------------------------

ifeq ($(MEMCHECK),1)
DEPS_BINDIR=$(ROOT)/bin/$(FULL_VARIANT)
DEPS_DEBUG=1
CMAKE_DEFS += MEMCHECK=ON
else
export DEPS_BINDIR:=$(ROOT)/bin/$(FULL_VARIANT.release)
DEPS_DEBUG=
endif

#----------------------------------------------------------------------------------------------

RAX_DIR = $(ROOT)/deps/rax
export RAX_BINDIR=$(DEPS_BINDIR)/rax
include $(ROOT)/build/rax/Makefile.defs

LIBXXHASH_DIR = $(ROOT)/deps/xxHash
export LIBXXHASH_BINDIR=$(DEPS_BINDIR)/xxHash
include $(ROOT)/build/xxHash/Makefile.defs

LIBCYPHER_PARSER_DIR = $(ROOT)/deps/libcypher-parser
LIBCYPHER_PARSER_SRCDIR = $(LIBCYPHER_PARSER_DIR)/lib/src
export LIBCYPHER_PARSER_BINDIR=$(DEPS_BINDIR)/libcypher-parser
include $(ROOT)/build/libcypher-parser/Makefile.defs

GRAPHBLAS_DIR = $(ROOT)/deps/GraphBLAS
export GRAPHBLAS_BINDIR=$(DEPS_BINDIR)/GraphBLAS
include $(ROOT)/build/GraphBLAS/Makefile.defs

REDISEARCH_DIR = $(ROOT)/deps/RediSearch
export REDISEARCH_BINROOT=$(BINROOT)
include $(ROOT)/build/RediSearch/Makefile.defs

BIN_DIRS += $(REDISEARCH_BINROOT)/search-static

LIBS=$(RAX) $(LIBXXHASH) $(GRAPHBLAS) $(REDISEARCH_LIBS) $(LIBCYPHER_PARSER)

#----------------------------------------------------------------------------------------------

CC_COMMON_H=$(SRCDIR)/src/common.h

include $(MK)/defs

$(info # Building into $(BINDIR))
$(info # Using CC=$(CC))

ifeq ($(UNIT_TESTS),1)
CMAKE_DEFS += UNIT_TESTS:BOOL=on
endif

#----------------------------------------------------------------------------------------------

MISSING_DEPS:=

ifeq ($(wildcard $(RAX)),)
MISSING_DEPS += $(RAX)
endif

ifeq ($(wildcard $(LIBXXHASH)),)
MISSING_DEPS += $(LIBXXHASH)
endif

ifeq ($(wildcard $(GRAPHBLAS)),)
MISSING_DEPS += $(GRAPHBLAS)
endif

ifeq ($(wildcard $(LIBCYPHER_PARSER)),)
MISSING_DEPS += $(LIBCYPHER_PARSER)
endif

ifneq ($(call files_missing,$(REDISEARCH_LIBS)),)
MISSING_DEPS += $(REDISEARCH_LIBS)
endif

ifneq ($(MISSING_DEPS),)
DEPS=1
endif

DEPENDENCIES=libcypher-parser graphblas redisearch rax libxxhash

ifneq ($(filter all deps $(DEPENDENCIES) pack,$(MAKECMDGOALS)),)
DEPS=1
endif

.PHONY: deps $(DEPENDENCIES)

#----------------------------------------------------------------------------------------------

RUN_CMD=redis-server --loadmodule $(abspath $(TARGET))

#----------------------------------------------------------------------------------------------

MK_ALL_TARGETS=bindirs deps build

.PHONY: all deps clean lint format pack run tests unit_tests flow_tests docker bindirs

all: bindirs $(TARGET)

include $(MK)/rules

#----------------------------------------------------------------------------------------------

ifeq ($(DEPS),1)

deps: $(LIBCYPHER_PARSER) $(GRAPHBLAS) $(LIBXXHASH) $(RAX) $(REDISEARCH_LIBS)

libxxhash: $(LIBXXHASH)

$(LIBXXHASH):
	@echo Building $@ ...
	$(SHOW)$(MAKE) --no-print-directory -C $(ROOT)/build/xxHash DEBUG=$(DEPS_DEBUG)

rax: $(RAX)

$(RAX):
	@echo Building $@ ...
	$(SHOW)$(MAKE) --no-print-directory -C $(ROOT)/build/rax DEBUG=$(DEPS_DEBUG)

graphblas: $(GRAPHBLAS)

$(GRAPHBLAS):
	@echo Building $@ ...
	$(SHOW)$(MAKE) --no-print-directory -C $(ROOT)/build/GraphBLAS DEBUG=$(DEPS_DEBUG)

libcypher-parser: $(LIBCYPHER_PARSER)

$(LIBCYPHER_PARSER):
	@echo Building $@ ...
	$(SHOW)$(MAKE) --no-print-directory -C $(ROOT)/build/libcypher-parser DEBUG=$(DEPS_DEBUG)

redisearch: $(REDISEARCH_LIBS)

$(REDISEARCH_LIBS):
	@echo Building $@ ...
	$(SHOW)$(MAKE) -C $(REDISEARCH_DIR) STATIC=1 BINROOT=$(REDISEARCH_BINROOT) CC=$(CC) CXX=$(CXX)

.PHONY: libcypher-parser graphblas redisearch libxxhash rax

#----------------------------------------------------------------------------------------------

else

deps: ;

endif # DEPS

.PHONY: deps

#----------------------------------------------------------------------------------------------

$(LIBCYPHER_PARSER_BINDIR)/lib/src/cypher-parser.h : $(LIBCYPHER_PARSER)

#----------------------------------------------------------------------------------------------

clean:
ifeq ($(ALL),1)
	$(SHOW)-rm -rf $(BINROOT) $(DEPS_BINDIR)
	$(SHOW)$(MAKE) -C $(ROOT)/build/libcypher-parser clean ALL=1
else
	$(SHOW)$(MAKE) -C $(BINDIR) clean
	$(SHOW)-rm -fr $(TARGET).debug $(BINDIR)/CMakeCache.txt $(BINDIR)/tests
ifeq ($(DEPS),1)
	$(SHOW)$(MAKE) -C $(ROOT)/build/rax clean DEBUG=$(DEPS_DEBUG)
	$(SHOW)$(MAKE) -C $(ROOT)/build/xxHash clean DEBUG=$(DEPS_DEBUG)
	$(SHOW)$(MAKE) -C $(ROOT)/build/GraphBLAS clean DEBUG=$(DEPS_DEBUG)
	$(SHOW)$(MAKE) -C $(ROOT)/build/libcypher-parser clean DEBUG=$(DEPS_DEBUG)
	$(SHOW)$(MAKE) -C $(REDISEARCH_DIR) clean ALL=1 BINROOT=$(REDISEARCH_BINROOT)
endif
endif

clean-libcypher-parser:
	$(SHOW)$(MAKE) -C $(ROOT)/build/libcypher-parser clean ALL=1 AUTOGEN=1

clean-search:
ifeq ($(ALL),1)
	$(SHOW)rm -rf $(REDISEARCH_BINROOT)/search-static
else
	$(SHOW)$(MAKE) -C $(REDISEARCH_DIR) clean BINROOT=$(REDISEARCH_BINROOT)
endif

.PHONY: clean clean-libcypher-parser clean-search

#----------------------------------------------------------------------------------------------

pack package: $(TARGET)
	@MODULE=$(realpath $(TARGET)) $(ROOT)/sbin/pack.sh

upload-release:
	$(SHOW)RELEASE=1 ./sbin/upload-artifacts

upload-artifacts:
	$(SHOW)SNAPSHOT=1 ./sbin/upload-artifacts

.PHONY: pack package upload-artifacts upload-release

#----------------------------------------------------------------------------------------------

ifeq ($(SLOW),1)
_RLTEST_PARALLEL=0
else ifneq ($(PARALLEL),)
_RLTEST_PARALLEL=$(PARALLEL)
else
_RLTEST_PARALLEL=1
endif

ifneq ($(BUILD),0)
TEST_DEPS=$(TARGET)
endif

ifeq ($(VG_DOCKER),1)
test:
	@echo Building docker to run valgrind on macOS
	$(SHOW)docker build -f tests/Dockerfile -t macos_test_docker .
else
test: $(TEST_DEPS)
	$(SHOW)MODULE=$(TARGET) BINROOT=$(BINROOT) PARALLEL=$(_RLTEST_PARALLEL) ./tests/flow/tests.sh
endif

unit-tests:
ifneq ($(BUILD),0)
	$(SHOW)$(MAKE) build FORCE=1 UNIT_TESTS=1
endif
	$(SHOW)BINROOT=$(BINROOT) ./tests/unit/tests.sh

flow-tests: $(TEST_DEPS)
	$(SHOW)MODULE=$(TARGET) BINROOT=$(BINROOT) PARALLEL=$(_RLTEST_PARALLEL) GEN=$(GEN) AOF=$(AOF) TCK=0 ./tests/flow/tests.sh

tck-tests: $(TEST_DEPS)
	$(SHOW)MODULE=$(TARGET) BINROOT=$(BINROOT) PARALLEL=$(_RLTEST_PARALLEL) GEN=0 AOF=0 TCK=1 ./tests/flow/tests.sh
	
.PHONY: test unit-tests flow-tests tck-tests

#----------------------------------------------------------------------------------------------

ifneq ($(TIMEOUT),)
FUZZ_ARGS=-t $(TIMEOUT)
endif

fuzz fuzz-tests: $(TARGET)
	$(SHOW)cd tests/fuzz && ./process.py -m $(TARGET) $(FUZZ_ARGS)

.PHONY: fuzz fuzz-tests

#----------------------------------------------------------------------------------------------

ifneq ($(REMOTE),)
BENCHMARK_ARGS=run-remote
else
BENCHMARK_ARGS=run-local
endif

BENCHMARK_ARGS += --module_path $(TARGET) --required-module graph
ifneq ($(BENCHMARK),)
BENCHMARK_ARGS += --test $(BENCHMARK)
endif

benchmark: $(TARGET)
	$(SHOW)cd tests/benchmarks && redisbench-admin $(BENCHMARK_ARGS)

.PHONY: benchmark

#----------------------------------------------------------------------------------------------

COV_EXCLUDE_DIRS += \
	deps \
	src/util/sds \
	tests

COV_EXCLUDE+=$(foreach D,$(COV_EXCLUDE_DIRS),'$(realpath $(ROOT))/$(D)/*')

coverage:
	$(SHOW)$(MAKE) build COV=1
	$(SHOW)$(COVERAGE_RESET)
	-$(SHOW)$(MAKE) test COV=1
	$(SHOW)$(COVERAGE_COLLECT_REPORT)

.PHONY: coverage

#----------------------------------------------------------------------------------------------

docker:
	$(SHOW)$(MAKE) -C build/docker

ifneq ($(wildcard /w/*),)
SANBOX_ARGS += -v /w:/w
endif

sanbox:
	@docker run -it -v $(PWD):/build -w /build --cap-add=SYS_PTRACE --security-opt seccomp=unconfined $(SANBOX_ARGS) redisfab/clang:13-x64-bullseye bash

.PHONY: box sanbox
