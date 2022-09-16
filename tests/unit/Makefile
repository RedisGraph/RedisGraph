
ifneq ($(filter coverage show-cov upload-cov,$(MAKECMDGOALS)),)
override COV:=1
endif

ifeq ($(COV),1)
override DEBUG:=1
endif

ifneq ($(filter memcheck,$(MAKECMDGOALS)),)
override DEBUG:=1
overide MEMCHECK:=1
endif

MK_ALL_TARGETS=bindirs deps build test

#----------------------------------------------------------------------------------------------

ROOT=../..

MK.pyver:=3
include $(ROOT)/deps/readies/mk/main

SRCDIR=.
BINDIR=$(BINROOT)/tests/unit

#----------------------------------------------------------------------------------------------
# Paths to deps.

RAX_DIR = $(ROOT)/deps/rax
export RAX_BINDIR=$(ROOT)/bin/$(FULL_VARIANT.release)/rax
include $(ROOT)/build/rax/Makefile.defs

LIBXXHASH_DIR = $(ROOT)/deps/xxHash
export LIBXXHASH_BINDIR=$(ROOT)/bin/$(FULL_VARIANT.release)/xxHash
include $(ROOT)/build/xxHash/Makefile.defs

LIBCYPHER_PARSER_DIR = $(ROOT)/deps/libcypher-parser
LIBCYPHER_PARSER_SRCDIR = $(LIBCYPHER_PARSER_DIR)/lib/src
export LIBCYPHER_PARSER_BINDIR=$(ROOT)/bin/$(FULL_VARIANT.release)/libcypher-parser
include $(ROOT)/build/libcypher-parser/Makefile.defs

GRAPHBLAS_DIR = $(ROOT)/deps/GraphBLAS
export GRAPHBLAS_BINDIR=$(ROOT)/bin/$(FULL_VARIANT.release)/GraphBLAS
include $(ROOT)/build/GraphBLAS/Makefile.defs

REDISEARCH_DIR = $(ROOT)/deps/RediSearch
export REDISEARCH_BINROOT=$(BINROOT)
include $(ROOT)/build/RediSearch/Makefile.defs

GTEST_DIR=$(ROOT)/deps/googletest/googletest
export GOOGLETEST_BINDIR=$(ROOT)/bin/$(FULL_VARIANT.release)/googletest
include $(ROOT)/build/googletest/Makefile.defs

REDISGRAPH=$(ROOT)/bin/$(FULL_VARIANT)/redisgraph.so

LIBS=$(GRAPHBLAS) $(REDISEARCH) $(LIBXXHASH) $(LIBCYPHER_PARSER) $(RAX) $(GOOGLETEST) $(REDISGRAPH)

#----------------------------------------------------------------------------------------------

MK_CUSTOM_CLEAN=1

include $(MK)/defs

_SOURCES=\
	test_aggregate_functions.cpp \
	test_algebraic_expression.cpp \
	test_all_paths.cpp \
	test_arithmetic_expression.cpp \
	test_arr.cpp \
	test_bfs.cpp \
	test_cache.cpp \
	test_cron.cpp \
	test_datablock.cpp \
	test_detect_cycle.cpp \
	test_dfs.cpp \
	test_execution_plan_clone.cpp \
	test_filter_tree.cpp \
	test_graph.cpp \
	test_map.cpp \
	test_object_pool.cpp \
	test_pagerank.cpp \
	test_query_graph.cpp \
	test_range.cpp \
	test_record.cpp \
	test_referenced_entities.cpp \
	test_rg_matrix.cpp \
	test_rg_matrix_iter.cpp \
	test_thread_pools.cpp \
	test_value.cpp

SOURCES=$(addprefix $(SRCDIR)/,$(_SOURCES))
OBJECTS=$(patsubst $(SRCDIR)/%.cpp,$(BINDIR)/%.o,$(SOURCES))

CC_DEPS = $(patsubst $(SRCDIR)/%.cpp,$(BINDIR)/%.d,$(SOURCES))

TARGETS=$(patsubst %.o,%.run,$(OBJECTS))

CC_FLAGS += \
	-fPIC \
	-fvisibility=hidden \
	-MMD -MF $(@:.o=.d) \
	-I$(SRCDIR) \
	-I$(BINDIR) \
	-I$(ROOT)/deps \
	-I$(ROOT)/deps/RedisModulesSDK \
	-I$(ROOT)/src \
	-I$(ROOT)/deps/minunit \
	-I$(RAX_DIR) \
	-I$(LIBCYPHER_PARSER_DIR) \
	-I$(LIBCYPHER_PARSER_BINDIR)/lib/src \
	-I$(LIBXXHASH_DIR) \
	-I$(REDISEARCH_DIR)/src \
	-I$(GTEST_DIR)/include \
	-I$(GTEST_DIR)/include/gtest

LD_FLAGS += -ldl $(LD_FLAGS.coverage)

CXX_FLAGS += \
	-g \
	-pthread \
	-std=c++11 \
	-Wall \
	-Wextra \
	-Wno-unused-function \
	-Wno-sign-compare \
	-Wno-format \
	-Wno-write-strings

ifeq ($(OS),macos)
	ifeq ($(STATIC_OMP),1)
		LIBOMP_PREFIX:=$(shell brew --prefix libomp)
		LIBOMP=$(LIBOMP_PREFIX)/lib/libomp.a
	else
		LIBOMP=-lomp -L$(shell brew --prefix libomp)/lib -Wl,-no_compact_unwind
	endif
endif

ifeq ($(OS),linux)
CXX_FLAGS += -fopenmp
else
CXX_FLAGS += $(LIBOMP)
endif

#----------------------------------------------------------------------------------------------

MISSING_DEPS:=

ifeq ($(wildcard $(GOOGLETEST)),)
MISSING_DEPS += $(GOOGLETEST)
endif

ifneq ($(MISSING_DEPS),)
DEPS=1
endif

DEPENDENCIES=googletest

ifneq ($(filter all deps $(DEPENDENCIES) pack,$(MAKECMDGOALS)),)
DEPS=1
endif

.PHONY: deps $(DEPENDENCIES)

#----------------------------------------------------------------------------------------------

include $(MK)/rules

-include $(CC_DEPS)

all: $(TARGETS) test

$(BINDIR)/%.o: $(SRCDIR)/%.cpp
	@echo Compiling $<...
	$(SHOW)$(CXX) $(CC_FLAGS) $(CXX_FLAGS) -c $< -o $@

$(TARGETS): $(BINDIR)/%.run: $(BINDIR)/%.o $(LIBS)
	@echo Building $@ ...
	$(SHOW)$(CXX) -o $@ $(LD_FLAGS) $(CXX_FLAGS) $^

$(GOOGLETEST):
	@echo Building $@ ...
	$(SHOW)$(MAKE) --no-print-directory -C $(ROOT)/build/googletest DEBUG=''

build: $(TARGETS)

clean:
	$(SHOW)rm -f $(TARGETS) $(OBJECTS)

.PHONY: all build test clean

#----------------------------------------------------------------------------------------------

test: build
ifeq ($(V),1)
	$(SHOW)for t in $(TARGETS); do \
		echo Running $$t ...; \
		o1=$$(mktemp) ;\
		$$t 2>&1 >$$o1 || { cat $$o1; rm $$o1; exit 1; }; \
		rm $$o1 ; \
	done
else
	$(SHOW)for t in $(TARGETS); do \
		echo Running $$t ...; \
		$$t || exit 1; \
	done
endif

.PHONY: test
