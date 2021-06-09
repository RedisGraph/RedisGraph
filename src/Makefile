
ROOT:=$(realpath ..)

# find the OS
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')

ifneq ($(filter cov-upload,$(MAKECMDGOALS)),)
COV=1
endif

ifeq ($(COV),1)
export DEBUG ?= 1
endif

# Paths to deps.
RAX_DIR = $(ROOT)/deps/rax
XXHASH_DIR = $(ROOT)/deps/xxHash
GRAPHBLAS_DIR = $(ROOT)/deps/GraphBLAS
REDISEARCH_DIR = $(ROOT)/deps/RediSearch/src
LIBCYPHER-PARSER_DIR = $(ROOT)/deps/libcypher-parser/lib/src

# if DEBUG env var is set, we compile with "debug" cflags
DEBUGFLAGS = -g -ggdb -O3
ifeq ($(DEBUG),1)
	DEBUGFLAGS = -fno-omit-frame-pointer -g -ggdb -O0
	# Enable all assertions in debug mode
	DEBUGFLAGS += -DRG_DEBUG
endif

# Default CFLAGS
CFLAGS = \
	-Wall -Wno-unused-function -Wno-unused-variable -Wno-unused-result \
	-fPIC -fcommon -fopenmp \
	-D_GNU_SOURCE -DREDIS_MODULE_TARGET -DREDISMODULE_EXPERIMENTAL_API -DXXH_STATIC_LINKING_ONLY
CFLAGS += $(DEBUGFLAGS)

# Compile flags for linux / osx
ifeq ($(uname_S),Linux)
	SHOBJ_LDFLAGS ?= -Wl,-Bsymbolic,-Bsymbolic-functions -fopenmp -shared -ldl -lpthread
	export OS = Linux
else
	CFLAGS += -mmacosx-version-min=10.14
	SHOBJ_LDFLAGS ?= -mmacosx-version-min=10.14 -bundle -undefined dynamic_lookup -ldl -lpthread -fopenmp
	export OS = Mac
endif
SHOBJ_LDFLAGS += $(LDFLAGS)
export CFLAGS

#----------------------------------------------------------------------------------------------

ifeq ($(COV),1)

CFLAGS += --coverage
SHOBJ_LDFLAGS += --coverage
BINROOT=$(SOURCEDIR)

COV_INFO=$(ROOT)/cov/cov.info
COV_DIR=$(ROOT)/cov

COV_EXCLUDE += \
    '/Applications/*' \
    '/usr/*' \
	'$(ROOT)/deps/*'

define COVERAGE_RESET
$(SHOW)set -e ;\
echo "Starting coverage analysys." ;\
mkdir -p $(COV_DIR) ;\
lcov --directory $(BINROOT) --base-directory $(SOURCEDIR) -z > /dev/null 2>&1
endef

define COVERAGE_COLLECT
$(SHOW)set -e ;\
echo "Collecting coverage data ..." ;\
lcov --capture --directory $(BINROOT) --base-directory $(SOURCEDIR) --output-file $(COV_INFO) > /dev/null 2>&1 ;\
lcov -o $(COV_INFO) -r $(COV_INFO) $(COV_EXCLUDE) > /dev/null 2>&1
endef

define COVERAGE_REPORT
$(SHOW)set -e ;\
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
CC_SOURCES += $(wildcard $(SOURCEDIR)/serializers/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/serializers/encoder/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/serializers/encoder/*/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/serializers/decoders/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/serializers/decoders/current/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/serializers/decoders/current/*/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/serializers/decoders/prev/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/serializers/decoders/prev/*/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/GraphBLASExt/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/grouping/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/index/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/ast/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/ast/enrichment/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/resultset/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/resultset/formatters/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/schema/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/slow_log/*.c)
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
REDISEARCH=$(ROOT)/deps/RediSearch/build/libredisearch.a
LIBCYPHER-PARSER=$(ROOT)/deps/libcypher-parser/lib/src/.libs/libcypher-parser.a

# Compilation deps for the module
LIBS=$(RAX) $(GRAPHBLAS) $(REDISEARCH) $(LIBXXHASH) $(LIBCYPHER-PARSER)
MODULE= $(LIBS) $(CC_OBJECTS)

#----------------------------------------------------------------------------------------------

REDISGRAPH_CC=$(QUIET_CC)$(CC)

CCCOLOR="\033[34m"
SRCCOLOR="\033[33m"
ENDCOLOR="\033[0m"

ifndef V
QUIET_CC = @printf '    %b %b\n' $(CCCOLOR)CC$(ENDCOLOR) $(SRCCOLOR)$@$(ENDCOLOR) 1>&2;
endif

%.c: %.y

# Compile C file while generating a .d file for it
%.o: %.c
%.o: %.c
	$(REDISGRAPH_CC) $(CFLAGS) -I. -I$(GRAPHBLAS_DIR)/Include -I$(RAX_DIR) -I$(LIBCYPHER-PARSER_DIR) -I$(XXHASH_DIR) -I$(REDISEARCH_DIR) -c $< -o $@ -MMD -MF $(@:.o=.d)

all: redisgraph.so

# Include all dependency files for C files
-include $(CC_DEPS)

#----------------------------------------------------------------------------------------------

$(RAX):
	@$(MAKE) -C $(ROOT)/deps/rax -f $(ROOT)/build/rax/Makefile
.PHONY: $(RAX)

$(LIBXXHASH):
	@$(MAKE) -C $(ROOT)/deps/xxHash lib
.PHONY: $(LIBXXHASH)

# Build GraphBLAS only if library does not already exists.
$(GRAPHBLAS):
ifeq (,$(wildcard $(GRAPHBLAS)))
	@$(MAKE) -C $(ROOT)/deps/GraphBLAS CMAKE_OPTIONS="-DCMAKE_C_COMPILER='$(CC)'" static_only JOBS=$(shell $(ROOT)/deps/readies/bin/nproc)
endif
.PHONY: $(GRAPHBLAS)

$(REDISEARCH):
ifeq (,$(wildcard $(REDISEARCH)))
	cd $(ROOT)/deps/RediSearch; \
	mkdir -p build; \
	cd build; \
	cmake -DRS_FORCE_NO_GITVERSION=ON -DRS_BUILD_STATIC=ON ..; \
	make;
endif
.PHONY: $(REDISEARCH)

# Build libcypher-parser only if library does not already exists.
$(LIBCYPHER-PARSER):
ifeq (,$(wildcard $(LIBCYPHER-PARSER)))
	cd $(ROOT)/deps/libcypher-parser; \
	sh ./autogen.sh; \
	./configure --disable-shared;
	$(MAKE) CFLAGS="-O3 -fPIC -DYY_BUFFER_SIZE=1048576" clean check -C $(ROOT)/deps/libcypher-parser
endif
.PHONY: $(LIBCYPHER-PARSER)

#----------------------------------------------------------------------------------------------

define extract_symbols
objcopy --only-keep-debug $1 $1.debug
objcopy --strip-debug $1
objcopy --add-gnu-debuglink $1.debug $1
endef

# Build the module...
redisgraph.so: $(MODULE)
	$(REDISGRAPH_CC) -o $@ $(CC_OBJECTS) $(LIBS) $(SHOBJ_LDFLAGS) -lc -lm
ifeq ($(uname_S),Linux)
ifneq ($(DEBUG),1)
	$(call extract_symbols,$@)
endif
endif

clean:
	@find . -name '*.[oad]' -type f -delete
	@-rm -f redisgraph.so
ifeq ($(ALL),1)
	@$(MAKE) -C $(ROOT)/deps/rax clean
	@$(MAKE) -C $(ROOT)/deps/xxHash clean
	@$(MAKE) -C $(ROOT)/deps/GraphBLAS clean
	@$(MAKE) -C $(ROOT)/deps/libcypher-parser clean
	rm -rf $(ROOT)/deps/RediSearch/build/      # Directly delete the RediSearch build artifacts
endif
	@$(MAKE) -C $(ROOT)/tests clean
.PHONY: clean

#----------------------------------------------------------------------------------------------

package: redisgraph.so
	@$(ROOT)/sbin/pack.sh
	@VARIANT=rce $(ROOT)/sbin/pack.sh
.PHONY: package

#----------------------------------------------------------------------------------------------

test: redisgraph.so
	# check valgrind flag is not empty
ifneq ($(VALGRIND),)
	# valgrind is requested, check that host's os is not Linux
ifneq ($(OS),Linux)
	@echo building docker to run valgrind on MacOS
	@cd .. ;\
	docker build -f tests/Dockerfile -t mac_os_test_docker .
endif
endif
	@$(COVERAGE_RESET)
	@$(MAKE) -C $(ROOT)/tests test
	@$(COVERAGE_COLLECT_REPORT)

.PHONY: test

#----------------------------------------------------------------------------------------------

memcheck: CFLAGS += -fno-omit-frame-pointer -g -ggdb -O0 -D MEMCHECK
memcheck: SHOBJ_LDFLAGS += -u RediSearch_CleanupModule
memcheck: redisgraph.so
	@$(MAKE) -C $(ROOT)/tests memcheck

.PHONY: memcheck

#----------------------------------------------------------------------------------------------

benchmark: redisgraph.so
	@$(MAKE) -C $(ROOT)/tests benchmark

.PHONY: benchmark

#----------------------------------------------------------------------------------------------

ifeq ($(COV),1)
cov-upload:
	$(SHOW)bash -c "bash <(curl -s https://codecov.io/bash) -f $(COV_INFO)"
.PHONY: cov-upload
endif # COV

#----------------------------------------------------------------------------------------------

fuzz: redisgraph.so
	@$(MAKE) -C $(ROOT)/tests fuzz

.PHONY: fuzz
