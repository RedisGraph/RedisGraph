# find the OS
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')

# store absolute path to RedisGraph Makefile location
abs_path = $(shell pwd)

# Paths to deps.
RAX_DIR = ../deps/rax
XXHASH_DIR = ../deps/xxHash
REDISEARCH_DIR = ../deps/RediSearch/src
LIBCYPHER-PARSER_DIR = ../deps/libcypher-parser/lib/src

# if DEBUG env var is set, we compile with "debug" cflags
DEBUGFLAGS = -g -ggdb -O3 
ifeq ($(DEBUG), 1)
	DEBUGFLAGS = -g -ggdb -O0
endif
DEBUGFLAGS = -g -ggdb -O0

# Default CFLAGS
CFLAGS = \
	-Wall -Wno-unused-function -Wno-unused-variable -Wno-unused-result \
	-fPIC -std=gnu99 \
	-D_GNU_SOURCE -DREDIS_MODULE_TARGET -DREDISMODULE_EXPERIMENTAL_API -DXXH_STATIC_LINKING_ONLY
CFLAGS += $(DEBUGFLAGS)

# Compile flags for linux / osx
ifeq ($(uname_S),Linux)
	SHOBJ_LDFLAGS ?= -shared -Bsymbolic -Bsymbolic-functions -ldl -lpthread
else
	CFLAGS += -mmacosx-version-min=10.14
	SHOBJ_LDFLAGS ?= -mmacosx-version-min=10.14 -bundle -undefined dynamic_lookup -ldl -lpthread
endif
SHOBJ_LDFLAGS += $(LDFLAGS)

export CFLAGS

# Sources
SOURCEDIR=$(shell pwd -P)
CC_SOURCES = $(wildcard $(SOURCEDIR)/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/algorithms/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/time_funcs/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/entity_funcs/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/string_funcs/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/boolean_funcs/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/numeric_funcs/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/arithmetic/conditional_funcs/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/bulk_insert/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/commands/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/datatypes/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/execution_plan/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/execution_plan/ops/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/execution_plan/optimizations/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/filter_tree/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/graph/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/graph/entities/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/graph/serializers/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/graph/serializers/encoder/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/graph/serializers/decoders/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/graph/serializers/decoders/prev/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/graph/serializers/decoders/prev/*/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/GraphBLASExt/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/grouping/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/index/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/ast/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/resultset/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/resultset/formatters/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/schema/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/procedures/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/util/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/util/datablock/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/util/thpool/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/util/range/*.c)

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
	$(REDISGRAPH_CC) $(CFLAGS) -I$(RAX_DIR) -I$(LIBCYPHER-PARSER_DIR) -I$(XXHASH_DIR) -I$(REDISEARCH_DIR) -c $< -o $@ -MMD -MF $(@:.o=.d)

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
BLAS_CFLAGS = -fPIC -I $(abs_path)/util -include rmalloc.h -DREDIS_MODULE_TARGET -DGB_MALLOC=rm_malloc -DGB_CALLOC=rm_calloc -DGB_REALLOC=rm_realloc -DGB_FREE=rm_free -DGBCOMPACT
$(GRAPHBLAS):
ifeq (,$(wildcard $(GRAPHBLAS)))
	@$(MAKE) -C ../deps/GraphBLAS CMAKE_OPTIONS="-DUSER_POSIX=1 -DCMAKE_CXX_FLAGS=-std=c++11 -DCMAKE_C_FLAGS='$(BLAS_CFLAGS)'" static_only
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
	./autogen.sh; \
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
	@$(MAKE) -C ../deps/RediSearch clean
	@$(MAKE) -C ../deps/libcypher-parser clean
endif
	@$(MAKE) -C ../tests clean

package: redisgraph.so
	@mkdir -p ../build
	@ramp pack -m "`pwd`/../ramp_manifest.yml" -v -o "../build/redisgraph.{os}-{architecture}.latest.zip" "`pwd`/redisgraph.so"

test: redisgraph.so
	# check valgrind flag is not empty
ifneq ($(VALGRIND),)
	# valgrind is requested, check that host's os is not Linux
ifneq ($(uname_S),Linux)
	@echo building docker to run valgrind on MacOS
	@cd ../ ;\
	docker build -f ./tests/Dockerfile -t mac_os_test_docker .;
endif
endif
	@$(MAKE) -C ../tests test
