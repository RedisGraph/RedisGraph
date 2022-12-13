
ROOT:=$(realpath ..)

DEPS_DIR:=$(ROOT)/deps

override OS:=$(shell $(DEPS_DIR)/readies/bin/platform --os)
ARCH:=$(shell $(DEPS_DIR)/readies/bin/platform --arch)

export OS
export ARCH

JOBS ?= $(shell MAX_NPROC=8 $(ROOT)/deps/readies/bin/nproc)

ifneq ($(filter cov-upload,$(MAKECMDGOALS)),)
COV=1
endif

ifneq ($(filter memcheck,$(MAKECMDGOALS)),)
export DEBUG ?= 1
export MEMCHECK=1
endif

ifeq ($(COV),1)
export DEBUG ?= 1
endif

ifeq ($(DEBUG),1)
FLAVOR=debug
else
FLAVOR=release
endif

#----------------------------------------------------------------------------------------------

TARGET=redisgraph.so
export BINROOT=$(ROOT)/bin/$(OS)-$(ARCH)-$(FLAVOR)

#----------------------------------------------------------------------------------------------

# Paths to deps.
RAX_DIR = $(ROOT)/deps/rax
XXHASH_DIR = $(ROOT)/deps/xxHash
GRAPHBLAS_DIR = $(ROOT)/deps/GraphBLAS
REDISEARCH_DIR = $(ROOT)/deps/RediSearch
REDISEARCH_BINROOT=$(BINROOT)
LIBCYPHER_PARSER_DIR = $(ROOT)/deps/libcypher-parser/lib/src

#----------------------------------------------------------------------------------------------

ifeq ($(CC),)
ifeq ($(GCC),1)
override CC=gcc
override CXX=g++
else ifeq ($(CLANG),1)
override CC=clang
override CXX=clang++
endif
endif

#----------------------------------------------------------------------------------------------

CFLAGS.warnings=-Wall -Wno-unused-variable -Wno-unused-result
ifeq ($(PEDANTIC),1)
CFLAGS.warnings += -Wpedantic
endif

# Default CFLAGS
CFLAGS=\
	-fPIC \
	-fcommon \
	-D_GNU_SOURCE \
	-DREDIS_MODULE_TARGET \
	-DREDISMODULE_EXPERIMENTAL_API \
	-DXXH_STATIC_LINKING_ONLY \
	$(CFLAGS.opt) \
	$(CFLAGS.warnings) \
	$(CFLAGS.debug)

# Compile flags for linux / osx
LDFLAGS.linux=-Wl,-Bsymbolic,-Bsymbolic-functions -fopenmp -shared -ldl -lpthread
LDFLAGS.macos=-bundle -undefined dynamic_lookup -ldl -lpthread $(LIBOMP)

SHOBJ_LDFLAGS += $(LDFLAGS) $(LDFLAGS.$(OS))

# if DEBUG env var is set, we compile with "debug" cflags
ifeq ($(DEBUG),1)
	CFLAGS.opt += -O0
	CFLAGS.debug += -fno-omit-frame-pointer -g -ggdb
	# Enable all assertions in debug mode
	CFLAGS.debug += -DRG_DEBUG
	ifeq ($(MEMCHECK),1)
		CFLAGS.debug += -DMEMCHECK
		SHOBJ_LDFLAGS += -u RediSearch_CleanupModule
	endif
else
	CFLAGS.opt += -O3
	CFLAGS.debug += -g -ggdb
endif

export CFLAGS

#----------------------------------------------------------------------------------------------

ifeq ($(COV),1)

CFLAGS += --coverage
SHOBJ_LDFLAGS += --coverage
COV_BINROOT=$(SOURCEDIR)

COV_INFO=$(ROOT)/cov/cov.info
COV_DIR=$(ROOT)/cov

COV_EXCLUDE += \
    '/Applications/*' \
    '/usr/*' \
	'$(ROOT)/deps/*' \
	'$(ROOT)/src/util/sds/*'

define COVERAGE_RESET
set -e ;\
echo "Starting coverage analysys ..." ;\
mkdir -p $(COV_DIR) ;\
lcov --directory $(COV_BINROOT) --base-directory $(SOURCEDIR) -z > /dev/null 2>&1
endef

define COVERAGE_COLLECT
set -e ;\
echo "Collecting coverage data ..." ;\
lcov --capture --directory $(COV_BINROOT) --base-directory $(SOURCEDIR) --output-file $(COV_INFO) > /dev/null 2>&1 ;\
lcov -o $(COV_INFO) -r $(COV_INFO) $(COV_EXCLUDE) > /dev/null 2>&1
endef

define COVERAGE_REPORT
set -e ;\
lcov -l $(COV_INFO) ;\
genhtml --legend -o $(COV_DIR) $(COV_INFO) > /dev/null 2>&1 ;\
echo "Coverage info at $$(realpath $(COV_DIR))/index.html"
endef

define COVERAGE_COLLECT_REPORT
$(COVERAGE_COLLECT)
$(COVERAGE_REPORT)
endef

else

COVERAGE_RESET=
COVERAGE_COLLECT=
COVERAGE_REPORT=
COVERAGE_COLLECT_REPORT=

endif # COV 

#----------------------------------------------------------------------------------------------

# Sources
SOURCEDIR:=$(shell pwd -P)
CC_SOURCES = $(wildcard $(SOURCEDIR)/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/configuration/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/algorithms/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/algorithms/LAGraph/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/map_funcs/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/path_funcs/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/list_funcs/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/time_funcs/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/point_funcs/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/entity_funcs/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/string_funcs/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/boolean_funcs/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/numeric_funcs/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/aggregate_funcs/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/conditional_funcs/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/placeholder_funcs/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/comprehension_funcs/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/algebraic_expression/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/bulk_insert/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/commands/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/datatypes/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/datatypes/path/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/execution_plan/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/execution_plan/ops/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/execution_plan/ops/shared/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/execution_plan/optimizations/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/execution_plan/execution_plan_build/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/filter_tree/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/graph/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/graph/entities/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/graph/rg_matrix/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/serializers/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/serializers/encoder/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/serializers/encoder/*/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/serializers/decoders/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/serializers/decoders/current/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/serializers/decoders/current/*/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/serializers/decoders/prev/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/serializers/decoders/prev/*/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/grouping/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/index/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/constraint/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/ast/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/ast/enrichment/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/resultset/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/resultset/formatters/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/schema/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/slow_log/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/undo_log/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/procedures/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/util/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/util/sds/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/util/datablock/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/util/object_pool/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/util/thpool/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/util/range/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/util/cache/*.c)


# Convert all sources to .o files
CC_OBJECTS = $(patsubst %.c, %.o, $(CC_SOURCES) )
export CC_OBJECTS

# .d files for each c file. These make sure that changing a header file
# will also change the dependent .c files of it
CC_DEPS = $(patsubst %.c, %.d, $(CC_SOURCES) )

#----------------------------------------------------------------------------------------------

# Library dependencies
RAX=$(ROOT)/deps/rax/rax.o
LIBXXHASH=$(ROOT)/deps/xxHash/libxxhash.a
GRAPHBLAS=$(ROOT)/deps/GraphBLAS/build/libgraphblas.a
REDISEARCH=$(REDISEARCH_BINROOT)/search-static/libredisearch.a
LIBCYPHER_PARSER=$(ROOT)/deps/libcypher-parser/lib/src/.libs/libcypher-parser.a

# Compilation deps for the module
LIBS=$(RAX) $(GRAPHBLAS) $(REDISEARCH) $(LIBXXHASH) $(LIBCYPHER_PARSER)
MODULE_DEPS=$(LIBS) $(CC_OBJECTS)

#----------------------------------------------------------------------------------------------

define REDISGRAPH_CC
$(QUIET_CC); $(CC)
endef

CCCOLOR="\033[34m"
SRCCOLOR="\033[33m"
ENDCOLOR="\033[0m"

ifndef V
define QUIET_CC
@printf '    %b %b\n' $(CCCOLOR)CC$(ENDCOLOR) $(SRCCOLOR)$@$(ENDCOLOR) 1>&2
endef
endif

%.c: %.y

# Compile C file while generating a .d file for it
%.o: %.c $(LIBCYPHER_PARSER_DIR)/cypher-parser.h
	$(REDISGRAPH_CC) $(CFLAGS) -I. -I$(GRAPHBLAS_DIR)/Include -I$(RAX_DIR) -I$(LIBCYPHER_PARSER_DIR) -I$(XXHASH_DIR) -I$(REDISEARCH_DIR)/src -c $< -o $@ -MMD -MF $(@:.o=.d)

all: $(TARGET)

.PHONY: all

# Include all dependency files for C files
-include $(CC_DEPS)

#----------------------------------------------------------------------------------------------

$(RAX):
	@$(MAKE) -C $(ROOT)/deps/rax -f $(ROOT)/build/rax/Makefile
# .PHONY: $(RAX)

$(LIBXXHASH):
	@$(MAKE) -C $(ROOT)/deps/xxHash lib

# Build GraphBLAS only if library does not already exists.
$(GRAPHBLAS):
ifeq (,$(wildcard $(GRAPHBLAS)))
	@$(MAKE) -C $(ROOT)/deps/GraphBLAS CMAKE_OPTIONS="-DCMAKE_C_COMPILER='$(CC)'" static_only JOBS=$(JOBS)
endif

$(REDISEARCH):
ifeq (,$(wildcard $(REDISEARCH)))
	make -C $(REDISEARCH_DIR) STATIC=1 BINROOT=$(REDISEARCH_BINROOT)
endif

# Build libcypher-parser only if library does not already exists.
$(LIBCYPHER_PARSER):
ifeq (,$(wildcard $(LIBCYPHER_PARSER)))
	@set -e ;\
	cd $(ROOT)/deps/libcypher-parser ;\
	sh ./autogen.sh ;\
	./configure --disable-shared
	@$(MAKE) CFLAGS="-O3 -fPIC -DYY_BUFFER_SIZE=1048576" clean check -C $(ROOT)/deps/libcypher-parser
endif

$(LIBCYPHER_PARSER_DIR)/cypher-parser.h : $(LIBCYPHER_PARSER)

#----------------------------------------------------------------------------------------------

define extract_symbols
objcopy --only-keep-debug $1 $1.debug
objcopy --strip-debug $1
objcopy --add-gnu-debuglink $1.debug $1
endef

# Build the module...
$(TARGET): $(MODULE_DEPS)
	$(REDISGRAPH_CC) -o $@ $(CC_OBJECTS) $(LIBS) $(SHOBJ_LDFLAGS) -lc -lm
ifeq ($(OS),linux)
ifneq ($(DEBUG),1)
	$(call extract_symbols,$@)
endif
endif

dynamic: $(MODULE_DEPS)
	$(REDISGRAPH_CC) -o $@ $(CC_OBJECTS) $(LIBS) -W -fopenmp -dynamiclib -lpthread -o libredisgraph.dylib
.PHONY: dynamic

clean:
	@find . -name '*.[oad]' -type f -delete
	@find . -name '*.gcda' -type f -delete
	@-rm -f $(TARGET)
ifeq ($(ALL),1)
	@$(MAKE) -C $(ROOT)/deps/rax clean
	@$(MAKE) -C $(ROOT)/deps/xxHash clean
	@$(MAKE) -C $(ROOT)/deps/GraphBLAS clean
	@$(MAKE) -C $(ROOT)/deps/libcypher-parser clean
	@cd $(ROOT)/deps/libcypher-parser; git ls-files -o | xargs rm
	@$(MAKE) -C $(REDISEARCH_DIR) clean ALL=1 BINROOT=$(REDISEARCH_BINROOT)
endif
	@$(MAKE) -C $(ROOT)/tests clean

.PHONY: clean

#----------------------------------------------------------------------------------------------

package: $(TARGET)
	@MODULE=$(realpath $(TARGET)) $(ROOT)/sbin/pack.sh

.PHONY: package

#----------------------------------------------------------------------------------------------

ifeq ($(SLOW),1)
_RLTEST_PARALLEL=0
else ifneq ($(PARALLEL),)
_RLTEST_PARALLEL=$(PARALLEL)
else
_RLTEST_PARALLEL=1
endif

test: $(TARGET)
# check valgrind flag is not empty
ifeq ($(VALGRIND),1)
# valgrind is requested, check that host's os is not Linux
ifeq ($(OS),macos)
	@echo building docker to run valgrind on MacOS
	@cd .. ;\
	docker build -f tests/Dockerfile -t mac_os_test_docker .
endif
endif
	@$(COVERAGE_RESET)
	@$(MAKE) -C $(ROOT)/tests test PARALLEL=$(_RLTEST_PARALLEL)
	@$(COVERAGE_COLLECT_REPORT)

.PHONY: test

#----------------------------------------------------------------------------------------------

memcheck: $(TARGET)
	@$(MAKE) -C $(ROOT)/tests memcheck PARALLEL=$(_RLTEST_PARALLEL)

.PHONY: memcheck

#----------------------------------------------------------------------------------------------

benchmark: $(TARGET)
	@$(MAKE) -C $(ROOT)/tests benchmark

.PHONY: benchmark

#----------------------------------------------------------------------------------------------

ifeq ($(COV),1)

cov-upload:
	$(SHOW)bash -c "bash <(curl -s https://codecov.io/bash) -f $(COV_INFO)"

.PHONY: cov-upload

endif # COV

#----------------------------------------------------------------------------------------------

fuzz: $(TARGET)
	@$(MAKE) -C $(ROOT)/tests fuzz

.PHONY: fuzz
