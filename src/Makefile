# find the OS
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')

# Paths to deps.
RAX_DIR = ../deps/rax
XXHASH_DIR = ../deps/xxHash
REDISEARCH_DIR = ../deps/RediSearch/src
LIBCYPHER-PARSER_DIR = ../deps/libcypher-parser/lib/src

# if DEBUG env var is set, we compile with "debug" cflags
DEBUGFLAGS = -g -ggdb -O3
ifeq ($(DEBUG), 1)
	DEBUGFLAGS = -fno-omit-frame-pointer -g -ggdb -O0
endif

# Default CFLAGS
CFLAGS = \
	-Wall -Wno-unused-function -Wno-unused-variable -Wno-unused-result \
	-fPIC -fcommon \
	-D_GNU_SOURCE -DREDIS_MODULE_TARGET -DREDISMODULE_EXPERIMENTAL_API -DXXH_STATIC_LINKING_ONLY
CFLAGS += $(DEBUGFLAGS)

# Compile flags for linux / osx
ifeq ($(uname_S),Linux)
	SHOBJ_LDFLAGS ?= -shared -Bsymbolic -Bsymbolic-functions -ldl -lpthread -fopenmp
	export OS = Linux
else
	CFLAGS += -mmacosx-version-min=10.14
	SHOBJ_LDFLAGS ?= -mmacosx-version-min=10.14 -bundle -undefined dynamic_lookup -ldl -lpthread -fopenmp
	export OS = Mac
endif
SHOBJ_LDFLAGS += $(LDFLAGS)
export CFLAGS

# Sources
SOURCEDIR=$(shell pwd -P)
CC_SOURCES = $(wildcard $(SOURCEDIR)/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/algorithms/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/path_funcs/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/list_funcs/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/placeholder_funcs/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/time_funcs/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/entity_funcs/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/string_funcs/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/boolean_funcs/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/numeric_funcs/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/conditional_funcs/*.c)
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

# Library dependencies
RAX=../deps/rax/rax.o
LIBXXHASH=../deps/xxHash/libxxhash.a
GRAPHBLAS=../deps/GraphBLAS/build/libgraphblas.a
REDISEARCH=../deps/RediSearch/build/libredisearch.a
LIBCYPHER-PARSER=../deps/libcypher-parser/lib/src/.libs/libcypher-parser.a

# Compilation deps for the module
LIBS=$(RAX) $(GRAPHBLAS) $(REDISEARCH) $(LIBXXHASH) $(LIBCYPHER-PARSER)
MODULE= $(LIBS) $(CC_OBJECTS)

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
	$(REDISGRAPH_CC) $(CFLAGS) -I. -I$(RAX_DIR) -I$(LIBCYPHER-PARSER_DIR) -I$(XXHASH_DIR) -I$(REDISEARCH_DIR) -c $< -o $@ -MMD -MF $(@:.o=.d)

all: redisgraph.so

# Include all dependency files for C files
-include $(CC_DEPS)

$(RAX):
	@$(MAKE) -C ../deps/rax
.PHONY: $(RAX)

$(LIBXXHASH):
	@$(MAKE) -C ../deps/xxHash lib
.PHONY: $(LIBXXHASH)

# Build GraphBLAS only if library does not already exists.
$(GRAPHBLAS):
ifeq (,$(wildcard $(GRAPHBLAS)))
	@$(MAKE) -C ../deps/GraphBLAS CMAKE_OPTIONS="-DCMAKE_C_COMPILER='$(CC)'" static_only
endif
.PHONY: $(GRAPHBLAS)

$(REDISEARCH):
ifeq (,$(wildcard $(REDISEARCH)))
	cd ../deps/RediSearch; \
	mkdir build; \
	cd build; \
	cmake -DRS_FORCE_NO_GITVERSION=ON -DRS_BUILD_STATIC=ON ..; \
	make;
endif
.PHONY: $(REDISEARCH)

# Build libcypher-parser only if library does not already exists.
$(LIBCYPHER-PARSER):
ifeq (,$(wildcard $(LIBCYPHER-PARSER)))
	cd ../deps/libcypher-parser; \
	sh ./autogen.sh; \
	./configure --disable-shared;
	$(MAKE) CFLAGS="-fPIC -DYY_BUFFER_SIZE=1048576" clean check -C ../deps/libcypher-parser
endif
.PHONY: $(LIBCYPHER-PARSER)

# Compile query parse.
# This is not included in the usual make target!
parser:
	@$(MAKE) -C $@
.PHONY: parser

# Build the module...
redisgraph.so: $(MODULE)
	$(REDISGRAPH_CC) -o $@ $(CC_OBJECTS) $(LIBS) $(SHOBJ_LDFLAGS) -lc -lm

clean:
	@find . -name '*.[oad]' -type f -delete
	@-rm -f redisgraph.so
ifeq ($(ALL),1)
	@$(MAKE) -C ../deps/rax clean
	@$(MAKE) -C ../deps/xxHash clean
	@$(MAKE) -C ../deps/GraphBLAS clean
	@$(MAKE) -C ../deps/libcypher-parser clean
	rm -rf ../deps/RediSearch/build/      # Directly delete the RediSearch build artifacts
endif
	@$(MAKE) -C ../tests clean

package: redisgraph.so
	@mkdir -p ../build
	@ramp pack -m "`pwd`/../ramp_manifest.yml" -v -o "../build/redisgraph.{os}-{architecture}.latest.zip" "`pwd`/redisgraph.so"

test: redisgraph.so
	# check valgrind flag is not empty
ifneq ($(VALGRIND),)
	# valgrind is requested, check that host's os is not Linux
ifneq ($(OS),Linux)
	@echo building docker to run valgrind on MacOS
	@cd ../ ;\
	docker build -f ./tests/Dockerfile -t mac_os_test_docker .;
endif
endif
	@$(MAKE) -C ../tests test

memcheck: CFLAGS += -fno-omit-frame-pointer -g -ggdb -O0 -D MEMCHECK
memcheck: SHOBJ_LDFLAGS += -u RediSearch_CleanupModule
memcheck: redisgraph.so
	@$(MAKE) -C ../tests memcheck
