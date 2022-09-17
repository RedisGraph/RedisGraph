
ifneq ($(filter memcheck,$(MAKECMDGOALS)),)
export DEBUG ?= 1
export MEMCHECK=1
endif

ifneq ($(SAN),)
override DEBUG ?= 1

ifeq ($(SAN),mem)
override SAN=memory
else ifeq ($(SAN),addr)
override SAN=address
endif

ifeq ($(SAN),address)
else ifeq ($(SAN),memory)
else ifeq ($(SAN),leak)
else ifeq ($(SAN),thread)
else
$(error SAN=mem|addr|leak|thread)
endif

export SAN
endif # SAN

#----------------------------------------------------------------------------------------------

ROOT=..

# MK.pyver:=3
include $(ROOT)/deps/readies/mk/main

MK_CUSTOM_CLEAN=1

#----------------------------------------------------------------------------------------------

BINDIR=$(BINROOT)

export TARGET=$(BINROOT)/redisgraph.so

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

LIBS=$(RAX) $(LIBXXHASH) $(GRAPHBLAS) $(REDISEARCH) $(LIBCYPHER_PARSER)

#----------------------------------------------------------------------------------------------

ifeq ($(OS),macos)
ifneq ($(GCC),1)
CLANG=1
endif
endif

#----------------------------------------------------------------------------------------------

CC_FLAGS.warnings=\
	-Wall \
	-Wno-unused-variable \
	-Wno-unused-result

ifeq ($(PEDANTIC),1)
CC_FLAGS.warnings += -Wpedantic
endif

CC_FLAGS=\
	-MMD -MF $(@:.o=.d) \
	-fPIC \
	-fcommon \
	-D_GNU_SOURCE \
	-DREDIS_MODULE_TARGET \
	-DREDISMODULE_EXPERIMENTAL_API \
	-DXXH_STATIC_LINKING_ONLY \
	$(CC_FLAGS.opt) \
	$(CC_FLAGS.warnings) \
	$(CC_FLAGS.debug) \
	$(CC_FLAGS.coverage) \
	$(CC_FLAGS.$(OS)) \
	-I. \
	-Igraph \
	-Iutil/cache \
	-Iutil/thpool \
	-I$(GRAPHBLAS_DIR)/Include \
	-I$(RAX_DIR) \
	-I$(LIBCYPHER_PARSER_SRCDIR) \
	-I$(LIBCYPHER_PARSER_BINDIR)/lib/src \
	-I$(LIBXXHASH_DIR) \
	-I$(REDISEARCH_DIR)/src

CC_FLAGS.macos += -mmacosx-version-min=10.15

ifeq ($(OS),macos)
	ifeq ($(STATIC_OMP),1)
		LIBOMP_PREFIX:=$(shell brew --prefix libomp)
		LIBOMP=$(LIBOMP_PREFIX)/lib/libomp.a
	else
		LIBOMP=-lomp -L$(shell brew --prefix libomp)/lib -Wl,-no_compact_unwind
	endif
endif

LD_FLAGS += \
	-lc \
	-lm \
	-ldl \
	-lpthread

LD_FLAGS.linux=\
	-Wl,-Bsymbolic,-Bsymbolic-functions \
	-fopenmp

LD_FLAGS.macos=\
	-undefined dynamic_lookup \
	$(LIBOMP)

SO_LD_FLAGS.linux=-shared
SO_LD_FLAGS.macos=-bundle

SO_LD_FLAGS += \
	$(SO_LD_FLAGS.$(OS)) \
	$(LD_FLAGS) \
	$(LD_FLAGS.$(OS)) \
	$(LD_FLAGS.coverage)

DYLIB_LD_FLAGS += \
	-dynamiclib \
	$(LD_FLAGS) \
	$(LD_FLAGS.$(OS)) \
	$(LD_FLAGS.coverage) \
	-static

ifeq ($(DEBUG),1)
	CC_FLAGS.opt += -O0
	CC_FLAGS.debug += -fno-omit-frame-pointer -g -ggdb
	# Enable all assertions in debug mode
	CC_FLAGS.debug += -DRG_DEBUG
	ifeq ($(MEMCHECK),1)
		CC_FLAGS.debug += -DMEMCHECK
		# SO_LD_FLAGS += -u RediSearch_CleanupModule
	endif
else
	CC_FLAGS.opt += -O3
	CC_FLAGS.debug += -g -ggdb
endif

#----------------------------------------------------------------------------------------------

SRCDIR=.

define SRC_SUBDIRS:=
	configuration
	algorithms
	arithmetic algorithms/LAGraph arithmetic/map_funcs arithmetic/path_funcs arithmetic/list_funcs
		arithmetic/time_funcs arithmetic/point_funcs arithmetic/entity_funcs arithmetic/string_funcs
		arithmetic/boolean_funcs arithmetic/numeric_funcs arithmetic/aggregate_funcs
		arithmetic/conditional_funcs arithmetic/placeholder_funcs arithmetic/comprehension_funcs
		arithmetic/algebraic_expression
	bulk_insert
	commands
	datatypes
	datatypes/path
	execution_plan execution_plan/ops execution_plan/ops/shared execution_plan/optimizations
		execution_plan/execution_plan_build
	filter_tree
	graph graph/entities graph/rg_matrix
	serializers serializers/encoder serializers/encoder/* serializers/decoders serializers/decoders/current
		serializers/decoders/current/* serializers/decoders/prev serializers/decoders/prev/*
	grouping
	index
	ast ast/enrichment
	resultset resultset/formatters
	schema
	slow_log
	undo_log
	procedures
	util util/sds util/datablock util/object_pool util/thpool util/range util/cache
endef

# _SOURCES:=$(wildcard *.c) $(foreach d,$(SRC_SUBDIRS),$(wildcard $(d)/*.c))
_SOURCES:=$(shell find . -name "*.c" | cut -f2- -d/ | sort)

SOURCES=$(addprefix $(SRCDIR)/,$(_SOURCES))
OBJECTS=$(patsubst %.c,$(BINDIR)/src/%.o,$(_SOURCES))

export REDISGRAPH_OBJECTS:=$(OBJECTS)

CC_DEPS = $(patsubst %.c,$(BINDIR)/src/%.d,$(_SOURCES))

include $(MK)/defs

$(info # Building into $(BINDIR))
$(info # Using CC=$(CC))

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

MK_ALL_TARGETS=bindirs deps build pack

.PHONY: all deps clean lint format pack run tests unit_tests flow_tests docker bindirs

all: bindirs $(TARGET)

include $(MK)/rules

#----------------------------------------------------------------------------------------------

CCCOLOR="\033[34m"
SRCCOLOR="\033[33m"
NOCOLOR="\033[0m"

-include $(CC_DEPS)

%.c: %.y

$(BINDIR)/src/%.o: $(ROOT)/src/%.c $(LIBCYPHER_PARSER_BINDIR)/lib/src/cypher-parser.h
	@printf '%b %b\n' $(CCCOLOR)CC$(NOCOLOR) $(SRCCOLOR)$<$(NOCOLOR)
	$(SHOW)$(CC) $(CC_FLAGS) -c $< -o $@

#----------------------------------------------------------------------------------------------

ifeq ($(DEPS),1)

.PHONY: libcypher-parser graphblas redisearch libxxhash rax

deps: $(LIBCYPHER_PARSER) $(GRAPHBLAS) $(LIBXXXHASH) $(RAX) $(REDISEARCH)

libxxhash: $(LIBXXHASH)

$(LIBXXHASH):
	@echo Building $@ ...
	$(SHOW)$(MAKE) --no-print-directory -C $(ROOT)/build/xxHash DEBUG=''

rax: $(RAX)

$(RAX):
	@echo Building $@ ...
	$(SHOW)$(MAKE) --no-print-directory -C $(ROOT)/build/rax DEBUG=''

graphblas: $(GRAPHBLAS)

$(GRAPHBLAS):
	@echo Building $@ ...
	$(SHOW)$(MAKE) --no-print-directory -C $(ROOT)/build/GraphBLAS DEBUG=''

libcypher-parser: $(LIBCYPHER_PARSER)

$(LIBCYPHER_PARSER):
	@echo Building $@ ...
	$(SHOW)$(MAKE) --no-print-directory -C $(ROOT)/build/libcypher-parser DEBUG=''

redisearch: $(REDISEARCH)

$(REDISEARCH):
	@echo Building $@ ...
	$(SHOW)$(MAKE) -C $(REDISEARCH_DIR) STATIC=1 BINROOT=$(REDISEARCH_BINROOT)

#----------------------------------------------------------------------------------------------

else

deps: ;

endif # DEPS

#----------------------------------------------------------------------------------------------

$(LIBCYPHER_PARSER_BINDIR)/lib/src/cypher-parser.h : $(LIBCYPHER_PARSER)

#----------------------------------------------------------------------------------------------

define extract_symbols
objcopy --only-keep-debug $1 $1.debug
objcopy --strip-debug $1
objcopy --add-gnu-debuglink $1.debug $1
endef

$(TARGET): $(LIBS) $(OBJECTS)
	@printf '%b %b\n' $(CCCOLOR)LINK$(NOCOLOR) $(SRCCOLOR)$@$(NOCOLOR)
	$(SHOW)$(CXX) -o $@ $(OBJECTS) $(LIBS) $(SO_LD_FLAGS)
ifeq ($(OS),macos)
	$(SHOW)$(CXX) -o $(patsubst %.so,%.dylib,$@) $(OBJECTS) $(LIBS) $(DYLIB_LD_FLAGS)
	@printf '%b %b\n' $(CCCOLOR)AR$(NOCOLOR) $(SRCCOLOR)$(patsubst %.so,%.a,$@)$(NOCOLOR)
	$(SHOW)$(AR) rcs $(patsubst %.so,%.a,$@) $(OBJECTS)
endif
ifeq ($(OS),linux)
ifneq ($(DEBUG),1)
	$(SHOW)$(call extract_symbols,$@)
endif
endif

clean:
ifeq ($(ALL),1)
	$(SHOW)-rm -rf $(BINDIR) $(ROOT)/bin/$(FULL_VARIANT.release)
	$(SHOW)$(MAKE) -C $(ROOT)/build/libcypher-parser clean ALL=1
else
	$(SHOW)-rm -f $(TARGET) $(OBJECTS) $(CC_DEPS)
ifeq ($(DEPS),1)
	$(SHOW)$(MAKE) -C $(ROOT)/build/rax clean
	$(SHOW)$(MAKE) -C $(ROOT)/build/xxHash clean
	$(SHOW)$(MAKE) -C $(ROOT)/build/GraphBLAS clean
	$(SHOW)$(MAKE) -C $(ROOT)/build/libcypher-parser clean
	$(SHOW)$(MAKE) -C $(REDISEARCH_DIR) clean ALL=1 BINROOT=$(REDISEARCH_BINROOT)
	$(SHOW)$(MAKE) -C $(ROOT)/tests clean
endif
endif

#----------------------------------------------------------------------------------------------

list-undefs:
	$(SHOW)nm -AC $(TARGET) | grep -v @@ | grep ' U '

list-bindeps:
ifneq ($(OS),macos)
	$(SHOW)ldd $(TARGET)
else
	$(SHOW)otool -L $(TARGET)
endif

.PHONY: clean list-undefs

#----------------------------------------------------------------------------------------------

pack package: $(TARGET)
	@MODULE=$(TARGET) $(ROOT)/sbin/pack.sh

upload-release:
	$(SHOW)RELEASE=1 ./sbin/upload-artifacts

upload-artifacts:
	$(SHOW)SNAPSHOT=1 ./sbin/upload-artifacts

.PHONY: pack upload-artifacts upload-release

#----------------------------------------------------------------------------------------------

run:
ifeq ($(GDB),1)
	$(SHOW)gdb -ex r --args redis-server --loadmodule $(abspath $(TARGET))
else
	$(SHOW)redis-server --loadmodule $(abspath $(TARGET))
endif

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
	@$(MAKE) -C $(ROOT)/tests test PARALLEL=$(_RLTEST_PARALLEL)

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
