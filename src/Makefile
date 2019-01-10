# find the OS
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')

# store absolute path to RedisGraph Makefile location
abs_path = $(shell pwd)

# if DEBUG env var is set, we compile with "debug" cflags
DEBUGFLAGS = -g -ggdb -O3 
ifeq ($(DEBUG), 1)
	DEBUGFLAGS = -g -ggdb -O0
endif

# Default CFLAGS
CFLAGS = \
	-Wall -Wno-unused-function -Wno-unused-variable -Wno-unused-result \
	-fPIC -std=gnu99 \
 	-I"$(abs_path)" -I"$(abs_path)/../deps" \
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
CC_SOURCES += $(wildcard $(SOURCEDIR)/bulk_insert/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/commands/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/execution_plan/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/execution_plan/ops/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/execution_plan/optimizations/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/filter_tree/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/graph/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/graph/entities/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/graph/serializers/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/GraphBLASExt/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/grouping/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/index/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/parser/clauses/*.c)
CC_SOURCES += $(SOURCEDIR)/parser/ast.c
CC_SOURCES += $(SOURCEDIR)/parser/newast.c
CC_SOURCES += $(SOURCEDIR)/parser/ast_common.c
CC_SOURCES += $(SOURCEDIR)/parser/ast_arithmetic_expression.c
CC_SOURCES += $(SOURCEDIR)/parser/lex.yy.c
CC_SOURCES += $(SOURCEDIR)/parser/grammar.c
CC_SOURCES += $(wildcard $(SOURCEDIR)/resultset/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/schema/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/util/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/util/datablock/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/util/thpool/*.c)

# Convert all sources to .o files
CC_OBJECTS = $(patsubst %.c, %.o, $(CC_SOURCES) )
export CC_OBJECTS

# .d files for each c file. These make sure that changing a header file
# will also change the dependent .c files of it
CC_DEPS = $(patsubst %.c, %.d, $(CC_SOURCES) )

# Library dependencies
LIBTRIEMAP=util/triemap/libtriemap.a
GRAPHBLAS=../deps/GraphBLAS/build/libgraphblas.a
LIBXXHASH=../deps/xxhash/libxxhash.a

# Compilation deps for the module
LIBS=$(LIBTRIEMAP) $(GRAPHBLAS) $(LIBXXHASH) $(LIBCYPHER-PARSER)
MODULE=$(CC_OBJECTS) $(LIBS)

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
	$(REDISGRAPH_CC) $(CFLAGS) -c $< -o $@ -MMD -MF $(@:.o=.d)

all: redisgraph.so

# Include all dependency files for C files
-include $(CC_DEPS)

$(LIBXXHASH):
	@$(MAKE) -C ../deps/xxhash lib

$(LIBTRIEMAP):
	@$(MAKE) -C util/triemap

BLAS_CFLAGS = -fPIC -I $(abs_path)/util -include rmalloc.h -DREDIS_MODULE_TARGET -DGB_MALLOC=rm_malloc -DGB_CALLOC=rm_calloc -DGB_REALLOC=rm_realloc -DGB_FREE=rm_free -DGBCOMPACT

# Build GraphBLAS only if library does not already exists.
$(GRAPHBLAS):
ifeq (,$(wildcard $(GRAPHBLAS)))
	@$(MAKE) -C ../deps/GraphBLAS CMAKE_OPTIONS="-DUSER_POSIX=1 -DCMAKE_CXX_FLAGS=-std=c++11 -DCMAKE_C_FLAGS='$(BLAS_CFLAGS)'" static_only
else

endif

# Build libcypher-parser only if library does not already exists.
$(LIBCYPHER-PARSER):
ifeq (,$(wildcard $(LIBCYPHER-PARSER)))
	# Note we're passing empty CFLAGS to libcypher-parser.
	cd ../deps/libcypher-parser; \
	./autogen.sh; \
	./configure;
	$(MAKE) clean check -C ../deps/libcypher-parser CFLAGS=
endif
.PHONY: $(LIBCYPHER-PARSER)

# Compile query parse. 
# This is not included in the usual make target!
parser:
	@$(MAKE) -C $@
.PHONY: parser

# Build the module...
redisgraph.so: $(MODULE)
	$(REDISGRAPH_CC) -o $@ $(MODULE) $(SHOBJ_LDFLAGS) -lc -lm

clean:
	@find . -name '*.[oad]' -type f -delete
	@-rm -f redisgraph.so
ifeq ($(ALL),1)
	@$(MAKE) -C ../deps/GraphBLAS clean
	@$(MAKE) -C ../deps/xxhash clean
endif
	@$(MAKE) -C ../tests clean

package: redisgraph.so
	@mkdir -p ../build
	@ramp pack -m "`pwd`/../ramp_manifest.yml" -v -o "../build/redisgraph.{os}-{architecture}.latest.zip" "`pwd`/redisgraph.so"

test: redisgraph.so
	@$(MAKE) -C ../tests test

test_valgrind: redisgraph.so
	# Run unit tests under Valgrind
	@$(MAKE) -C ../tests test_valgrind
