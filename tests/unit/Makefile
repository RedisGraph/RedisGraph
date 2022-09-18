
ifneq ($(filter memcheck,$(MAKECMDGOALS)),)
override DEBUG:=1
overide MEMCHECK:=1
endif

MK_ALL_TARGETS=bindirs deps build test

#----------------------------------------------------------------------------------------------

ROOT=../..

include $(ROOT)/deps/readies/mk/main

SRCDIR=.
BINDIR=$(BINROOT)/tests/unit

#----------------------------------------------------------------------------------------------
# Paths to deps.

ifeq ($(MEMCHECK),1)
DEPS_BINDIR?=$(ROOT)/bin/$(FULL_VARIANT)
DEPS_DEBUG?=1
else
export DEPS_BINDIR?=$(ROOT)/bin/$(FULL_VARIANT.release)
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

GTEST_DIR=$(ROOT)/deps/googletest/googletest
export GOOGLETEST_BINDIR=$(DEPS_BINDIR)/googletest
include $(ROOT)/build/googletest/Makefile.defs

ifeq ($(OS),macos)
REDISGRAPH=$(ROOT)/bin/$(FULL_VARIANT)/redisgraph.a
else
REDISGRAPH=$(ROOT)/bin/$(FULL_VARIANT)/redisgraph.so
endif

ifeq ($(OS),macos)
LIBS=$(REDISGRAPH_OBJECTS) $(GRAPHBLAS) $(REDISEARCH) $(LIBXXHASH) $(LIBCYPHER_PARSER) $(RAX) $(GOOGLETEST)
else
LIBS=$(GRAPHBLAS) $(LIBXXHASH) $(LIBCYPHER_PARSER) $(RAX) $(REDISEARCH) $(REDISGRAPH) $(GOOGLETEST)
endif

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
OBJECTS=$(patsubst %.cpp,$(BINDIR)/%.o,$(_SOURCES))

CC_DEPS = $(patsubst %.cpp,$(BINDIR)/%.d,$(_SOURCES))

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

LD_FLAGS += \
	-ldl \
	$(LD_FLAGS.coverage)

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

CXX_FLAGS.linux += -fopenmp
CXX_FLAGS.macos += $(LIBOMP) -mmacosx-version-min=10.15
CXX_FLAGS += $(CXX_FLAGS.$(OS))

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
