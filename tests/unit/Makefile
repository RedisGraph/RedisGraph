
ROOT=../..

# Path to Google Test source
GTEST_DIR = ../../deps/googletest/googletest
RAX_DIR = ../../deps/rax
XXHASH_DIR = ../../deps/xxHash
REDISEARCH_DIR = ../../deps/RediSearch/src
LIBCYPHER-PARSER_DIR = ../../deps/libcypher-parser/lib/src

LDFLAGS += -ldl -lm

# Flags passed to the C compiler.
CFLAGS += -g -Wall -Wextra -pthread -fopenmp
C_SUPPRESS = -Wno-unused-function -Wno-sign-compare -Wno-format -Wno-write-strings

ifeq ($(COV),1)
CFLAGS += --coverage
LDFLAGS += --coverage
endif

# REDISGRAPH_CC=$(QUIET_CC)$(CC)
REDISGRAPH_CC=$(CC)

CCCOLOR="\033[34m"
SRCCOLOR="\033[33m"
ENDCOLOR="\033[0m"

ifndef V
QUIET_CC = @printf '    %b %b\n' $(CCCOLOR)CC$(ENDCOLOR) $(SRCCOLOR)$@$(ENDCOLOR) 1>&2;
endif

# RedisGraph flags and libraries
CC_OBJECTS:=$(CC_OBJECTS)
RAX=../../deps/rax/rax.o
LIBXXHASH=$(ROOT)/deps/xxHash/libxxhash.a
REDISEARCH=../../deps/RediSearch/build/libredisearch.a
LIBGRAPHBLAS=../../deps/GraphBLAS/build/libgraphblas.a
LIBCYPHER-PARSER=../../deps/libcypher-parser/lib/src/.libs/libcypher-parser.a

LIBS=$(LIBGRAPHBLAS) $(REDISEARCH) $(LIBXXHASH) $(LIBCYPHER-PARSER)
DEPS=$(CC_OBJECTS) $(RAX) $(LIBS)

# Build and run a test for each cpp file in directory
TEST_SOURCES = $(wildcard *.c)
TEST_OBJECTS = $(patsubst %.c, %.o, $(TEST_SOURCES))
TEST_EXECUTABLES = $(patsubst %.c, %.run, $(TEST_SOURCES))

# Compile object files from unit test sources
%.o: %.c
	@$(REDISGRAPH_CC) $(CPPFLAGS) $(CFLAGS) $(C_SUPPRESS) -I$(RAX_DIR) -I$(LIBCYPHER-PARSER_DIR) -I$(XXHASH_DIR) -I$(REDISEARCH_DIR) -c -o $@ $<

# Build '*.run' binaries for each source
%.run: %.o $(DEPS)
	@$(REDISGRAPH_CC) $(CPPFLAGS) $(CFLAGS) $(C_SUPPRESS) $^ $(LDFLAGS) -o $@


.PHONY: all build test clean

all: build test

build: $(TEST_OBJECTS) $(TEST_EXECUTABLES) $(DEPS)

test: build
ifeq ($(V),)
	@for t in $(TEST_EXECUTABLES); do \
		echo Running $$t ...; \
		o1=$$(mktemp) ;\
		./$$t 2>&1 >$$o1 || { cat $$o1; rm $$o1; exit 1; }; \
		rm $$o1 ; \
	done
else
	@for t in $(TEST_EXECUTABLES); do \
		echo Running $$t ...; \
		./$$t || exit 1; \
	done
endif

clean:
	@rm -f gtest.a gtest_main.a *.o *.run
