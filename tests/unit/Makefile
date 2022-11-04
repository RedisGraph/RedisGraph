
ROOT:=$(realpath ../..)

DEPS_DIR:=$(ROOT)/deps

override OS:=$(shell $(DEPS_DIR)/readies/bin/platform --os)
ARCH:=$(shell $(DEPS_DIR)/readies/bin/platform --arch)

export OS
export ARCH

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

# Flags passed to the preprocessor.
ifeq ($(OS),macos)
	LIBOMP=-lomp -L$(shell brew --prefix libomp)/lib
endif

ifeq ($(OS),linux)
	CFLAGS=-fopenmp
else
	CFLAGS=$(LIBOMP)
endif

ifeq ($(COV),1)
	CFLAGS += --coverage
endif

#-------------------------------------------------------------------------------

# Paths to deps.
RAX_DIR = $(ROOT)/deps/rax
XXHASH_DIR = $(ROOT)/deps/xxHash
GRAPHBLAS_DIR = $(ROOT)/deps/GraphBLAS
REDISEARCH_DIR = $(ROOT)/deps/RediSearch
LIBCYPHER_PARSER_DIR = $(ROOT)/deps/libcypher-parser/lib/src

#-------------------------------------------------------------------------------

# Build and run a test for each cpp file in directory
TEST_SOURCES = $(wildcard *.c)
TEST_OBJECTS = $(patsubst %.c, %.o, $(TEST_SOURCES))
TEST_EXECUTABLES = $(patsubst %.c, %.run, $(TEST_SOURCES))

# Include header directories
INCLUDE_DEPS = -I$(GRAPHBLAS_DIR)/Include -I$(REDISEARCH_DIR)/src -I$(RAX_DIR) -I$(XXHASH_DIR) -I$(LIBCYPHER_PARSER_DIR) 

# Build '*.run' binaries for each source
%.run: %.c
	@$(CC) $(INCLUDE_DEPS) $(CFLAGS) -L$(ROOT)/src -L$(GRAPHBLAS_DIR)/build -lredisgraph -lgraphblas $< -o $@

.PHONY: all build test test_valgrind clean

test: $(TEST_EXECUTABLES)
# create symbolic link to redisgraph.so
	@ln -f -s $(ROOT)/src/redisgraph.so ./redisgraph.so
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

all: test

clean:
	@rm -f *.o *.run
