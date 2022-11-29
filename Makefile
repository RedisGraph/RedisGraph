
.NOTPARALLEL:

ifneq ($(filter memcheck,$(MAKECMDGOALS)),)
export DEBUG ?= 1
export MEMCHECK=1
endif

#----------------------------------------------------------------------------------------------

ROOT=.

MK.cmake=1
SRCDIR=src

include $(ROOT)/deps/readies/mk/main

MK_ALL_TARGETS=bindirs deps build

MK_CUSTOM_CLEAN=1

BINDIR=$(BINROOT)/src
export TARGET=$(BINROOT)/src/redisgraph.so

#----------------------------------------------------------------------------------------------

define HELPTEXT
make all            # Build everything
  DEBUG=1             # Build for debugging
  COV=1               # Build for coverage analysis (implies DEBUG=1)
  SLOW=1              # Disable parallel build
  STATIC_OMP=1        # Link OpenMP statically
  VARIANT=name        # Add `name` to build products directory
  GCC=1               # Build with GCC toolchain (default nor Linux)
  CLANG=1             # Build with CLang toolchain (default for macOS)
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

#----------------------------------------------------------------------------------------------

ifeq ($(MEMCHECK),1)
DEPS_BINDIR=$(ROOT)/bin/$(FULL_VARIANT)
DEPS_DEBUG=1
else
export DEPS_BINDIR:=$(ROOT)/bin/$(FULL_VARIANT.release)
DEPS_DEBUG=
endif

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

ifeq ($(OS),macos)
ifneq ($(GCC),1)
CLANG=1
endif
endif

#----------------------------------------------------------------------------------------------

ifeq ($(DEBUG),1)
	# Enable all assertions in debug mode
	CC_FLAGS.debug += -DRG_DEBUG
	ifeq ($(MEMCHECK),1)
		CC_FLAGS.debug += -DMEMCHECK
		SO_LD_FLAGS += -u RediSearch_CleanupModule
	endif
endif

#----------------------------------------------------------------------------------------------

include $(MK)/defs

ifneq ($(filter all build,$(MAKECMDGOALS)),)
$(info # Building into $(BINDIR))
$(info # Using CC=$(CC))
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

ifeq ($(wildcard $(REDISEARCH)),)
MISSING_DEPS += $(REDISEARCH)
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

MK_ALL_TARGETS=bindirs deps build

.PHONY: all deps clean lint format pack run tests unit_tests flow_tests docker bindirs

all: bindirs $(TARGET)

ifeq ($(OS),linux)
ifneq ($(DEBUG),1)
EXTRACT_TARGET_SYMBOLS=1
endif
endif

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
#	$(SHOW)-rm -f $(TARGET) $(OBJECTS) $(CC_DEPS)
	$(SHOW)$(MAKE) -C $(BINDIR) clean
ifeq ($(DEPS),1)
	$(SHOW)$(MAKE) -C $(ROOT)/build/rax clean
	$(SHOW)$(MAKE) -C $(ROOT)/build/xxHash clean
	$(SHOW)$(MAKE) -C $(ROOT)/build/GraphBLAS clean
	$(SHOW)$(MAKE) -C $(ROOT)/build/libcypher-parser clean
	$(SHOW)$(MAKE) -C $(REDISEARCH_DIR) clean ALL=1 BINROOT=$(REDISEARCH_BINROOT)
	$(SHOW)$(MAKE) -C $(ROOT)/tests clean
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

.PHONY: clean

#----------------------------------------------------------------------------------------------

pack package: $(TARGET)
	@MODULE=$(realpath $(TARGET)) $(ROOT)/sbin/pack.sh

upload-release:
	$(SHOW)RELEASE=1 ./sbin/upload-artifacts

upload-artifacts:
	$(SHOW)SNAPSHOT=1 ./sbin/upload-artifacts

.PHONY: pack package upload-artifacts upload-release

#----------------------------------------------------------------------------------------------

RUN_CMD=redis-server --loadmodule $(abspath $(TARGET))

#----------------------------------------------------------------------------------------------

ifeq ($(SLOW),1)
_RLTEST_PARALLEL=0
else ifneq ($(PARALLEL),)
_RLTEST_PARALLEL=$(PARALLEL)
else
_RLTEST_PARALLEL=1
endif

test: $(TARGET)
ifeq ($(VALGRIND),1)
# valgrind is requested, check that host's os is not Linux
ifeq ($(OS),macos)
	@echo building docker to run valgrind on MacOS
	@cd .. ;\
	docker build -f tests/Dockerfile -t mac_os_test_docker .
endif
endif
	@$(MAKE) -C $(ROOT)/tests test PARALLEL=$(_RLTEST_PARALLEL) BINROOT=$(BINROOT)

.PHONY: test

#----------------------------------------------------------------------------------------------

memcheck: bindirs $(TARGET)
	@$(MAKE) -C $(ROOT)/tests memcheck PARALLEL=$(_RLTEST_PARALLEL)

.PHONY: memcheck

#----------------------------------------------------------------------------------------------

benchmark: $(TARGET)
	@$(MAKE) -C $(ROOT)/tests benchmark

.PHONY: benchmark

#----------------------------------------------------------------------------------------------

COV_EXCLUDE_DIRS += \
	deps \
	src/util/sds

COV_EXCLUDE+=$(foreach D,$(COV_EXCLUDE_DIRS),'$(realpath $(ROOT))/$(D)/*')

coverage:
	$(SHOW)$(MAKE) build COV=1
	$(SHOW)$(COVERAGE_RESET)
	-$(SHOW)$(MAKE) test COV=1
	$(SHOW)$(COVERAGE_COLLECT_REPORT)

.PHONY: coverage

#----------------------------------------------------------------------------------------------

fuzz: $(TARGET)
	@$(MAKE) -C $(ROOT)/tests fuzz

.PHONY: fuzz
