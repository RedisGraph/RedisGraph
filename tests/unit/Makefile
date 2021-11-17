
ROOT=../..

# Path to Google Test source
GTEST_DIR = ../../deps/googletest/googletest
RAX_DIR = ../../deps/rax
XXHASH_DIR = ../../deps/xxHash
REDISEARCH_DIR = ../../deps/RediSearch/src
LIBCYPHER-PARSER_DIR = ../../deps/libcypher-parser/lib/src
REDISGRAPH_DIR = ../../src
GRAPHBLAS_DIR = ../../deps/GraphBLAS/build

# Flags passed to the preprocessor.
# Set Google Test's header directory as a system directory, such that
# the compiler doesn't generate warnings in Google Test headers.
CPPFLAGS += -isystem $(GTEST_DIR)/include -I$(GTEST_DIR)/include/gtest/
LDFLAGS += -ldl

# Flags passed to the C++ compiler.
CXXFLAGS += -g -Wall -Wextra -pthread -std=c++11 -fopenmp
CXX_SUPPRESS = -Wno-unused-function -Wno-sign-compare -Wno-format -Wno-write-strings

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
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
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

# RedisGraph and dependency libraries
REDISGRAPH=redisgraph.so
LIBGRAPHBLAS=libgraphblas.a

# Build and run a test for each cpp file in directory
TEST_SOURCES = $(wildcard *.cpp)
TEST_OBJECTS = $(patsubst %.cpp, %.o, $(TEST_SOURCES))
TEST_EXECUTABLES = $(patsubst %.cpp, %.run, $(TEST_SOURCES))

# Compile object files from unit test sources
%.o: %.cpp
	@$(REDISGRAPH_CXX) $(CPPFLAGS) $(CXXFLAGS) $(CXX_SUPPRESS) -I$(RAX_DIR) -I$(LIBCYPHER-PARSER_DIR) -I$(XXHASH_DIR) -I$(REDISEARCH_DIR) -c -o $@ $<

# Build '*.run' binaries for each source
%.run: %.o gtest_main.a
	@$(REDISGRAPH_CXX) $(CPPFLAGS) $(CXXFLAGS) $(CXX_SUPPRESS) $^ -L $(REDISGRAPH_DIR) -l:$(REDISGRAPH) -L $(GRAPHBLAS_DIR) -l:$(LIBGRAPHBLAS) -Wl,-rpath=$(REDISGRAPH_DIR) $(LDFLAGS) -o $@


.PHONY: all build test clean

all: build test

build: $(TEST_OBJECTS) $(TEST_EXECUTABLES)

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
