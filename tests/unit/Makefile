
ROOT:=$(realpath ../..)

DEPS_DIR:=$(ROOT)/deps

override OS:=$(shell $(DEPS_DIR)/readies/bin/platform --os)
ARCH:=$(shell $(DEPS_DIR)/readies/bin/platform --arch)

export OS
export ARCH

ifeq ($(DEBUG),1)
FLAVOR=debug
else
FLAVOR=release
endif

# Path to Google Test source
GTEST_DIR = $(DEPS_DIR)/googletest/googletest
RAX_DIR = $(DEPS_DIR)/rax
XXHASH_DIR = $(DEPS_DIR)/xxHash
REDISEARCH_DIR = $(DEPS_DIR)/RediSearch
REDISEARCH_BINROOT=$(ROOT)/bin/$(OS)-$(ARCH)-$(FLAVOR)
LIBCYPHER_PARSER_DIR = $(DEPS_DIR)/libcypher-parser/lib/src

# Flags passed to the preprocessor.
# Set Google Test's header directory as a system directory, such that
# the compiler doesn't generate warnings in Google Test headers.
CPPFLAGS += -isystem $(GTEST_DIR)/include -I$(GTEST_DIR)/include/gtest/
LDFLAGS += -ldl

# Flags passed to the C++ compiler.
CXXFLAGS += -g -Wall -Wextra -pthread -std=c++11
CXX_SUPPRESS = -Wno-unused-function -Wno-sign-compare -Wno-format -Wno-write-strings

ifeq ($(OS),macos)
	ifeq ($(STATIC_OMP),1)
		LIBOMP_PREFIX:=$(shell brew --prefix libomp)
		LIBOMP=$(LIBOMP_PREFIX)/lib/libomp.a
	else
		LIBOMP=-lomp -L$(shell brew --prefix libomp)/lib -Wl,-no_compact_unwind
	endif
endif

ifeq ($(OS),linux)
CXXFLAGS += -fopenmp
else
CXXFLAGS += $(LIBOMP)
endif

ifeq ($(COV),1)
CXXFLAGS += --coverage
LDFLAGS += --coverage
endif

REDISGRAPH_CXX=$(QUIET_CXX)$(CXX)

CCCOLOR="\033[34m"
SRCCOLOR="\033[33m"
ENDCOLOR="\033[0m"

ifndef V
QUIET_CXX = @printf '    %b %b\n' $(CCCOLOR)CXX$(ENDCOLOR) $(SRCCOLOR)$@$(ENDCOLOR) 1>&2;
endif

# Google Test headers - do not modify
GTEST_HEADERS = \
	$(GTEST_DIR)/include/gtest/*.h \
	$(GTEST_DIR)/include/gtest/internal/*.h

# Google Test compiler targets
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

gtest-all.o : $(GTEST_SRCS_)
	@$(REDISGRAPH_CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest-all.cc

gtest_main.o : $(GTEST_SRCS_)
	@$(REDISGRAPH_CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest_main.cc

gtest.a : gtest-all.o
	@$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	@$(AR) $(ARFLAGS) $@ $^

# RedisGraph flags and libraries
CC_OBJECTS:=$(CC_OBJECTS)
RAX=$(DEPS_DIR)/rax/rax.o
LIBXXHASH=$(DEPS_DIR)/xxHash/libxxhash.a
REDISEARCH=$(REDISEARCH_BINROOT)/search-static/libredisearch.a
LIBGRAPHBLAS=$(DEPS_DIR)/GraphBLAS/build/libgraphblas.a
LIBCYPHER_PARSER=$(DEPS_DIR)/libcypher-parser/lib/src/.libs/libcypher-parser.a

LIBS=$(LIBGRAPHBLAS) $(REDISEARCH) $(LIBXXHASH) $(LIBCYPHER_PARSER)
DEPS=$(CC_OBJECTS) $(RAX) $(LIBS)

# Build and run a test for each cpp file in directory
TEST_SOURCES = $(wildcard *.cpp)
TEST_OBJECTS = $(patsubst %.cpp, %.o, $(TEST_SOURCES))
TEST_EXECUTABLES = $(patsubst %.cpp, %.run, $(TEST_SOURCES))

# Compile object files from unit test sources
%.o: %.cpp
	@$(REDISGRAPH_CXX) $(CPPFLAGS) $(CXXFLAGS) $(CXX_SUPPRESS) -I$(RAX_DIR) -I$(LIBCYPHER_PARSER_DIR) -I$(XXHASH_DIR) -I$(REDISEARCH_DIR)/src -c -o $@ $<

# Build '*.run' binaries for each source
%.run: %.o gtest_main.a $(DEPS)
	@$(REDISGRAPH_CXX) $(CPPFLAGS) $(CXXFLAGS) $(CXX_SUPPRESS) $^ $(LDFLAGS) -o $@


.PHONY: all build test test_valgrind clean

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
