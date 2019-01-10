
ROOT=../..

# Path to Google Test source
GTEST_DIR = ../../deps/googletest

# Flags passed to the preprocessor.
# Set Google Test's header directory as a system directory, such that
# the compiler doesn't generate warnings in Google Test headers.
CPPFLAGS += -isystem $(GTEST_DIR)/include

# Flags passed to the C++ compiler.
CXXFLAGS += -g -Wall -Wextra -pthread -std=c++11
CXX_SUPPRESS = -Wno-sign-compare -Wno-format -Wno-write-strings

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

# RedisGraph flags and libraries
CC_OBJECTS:=$(CC_OBJECTS)
LIBTRIEMAP=../../src/util/triemap/libtriemap.a
LIBGRAPHBLAS=../../deps/GraphBLAS/build/libgraphblas.a
LIBXXHASH=$(ROOT)/deps/xxhash/libxxhash.a

LIBCYPHER-PARSER=../../deps/libcypher-parser/lib/src/.libs/libcypher-parser.a
LIBS=$(LIBTRIEMAP) $(LIBGRAPHBLAS) $(LIBXXHASH) $(LIBCYPHER-PARSER)
DEPS=$(CC_OBJECTS) $(LIBS)

# Build and run a test for each cpp file in directory
TEST_SOURCES = $(wildcard *.cpp)
TEST_OBJECTS = $(patsubst %.cpp, %.o, $(TEST_SOURCES))
TEST_EXECUTABLES = $(patsubst %.cpp, %.run, $(TEST_SOURCES))

# Compile object files from unit test sources
%.o: %.cpp
	@$(REDISGRAPH_CXX) $(CPPFLAGS) $(CXXFLAGS) $(CXX_SUPPRESS) -c -o $@ $<

# Build '*.run' binaries for each source
%.run: %.o gtest_main.a $(DEPS)
	@$(REDISGRAPH_CXX) $(CPPFLAGS) $(CXXFLAGS) $(CXX_SUPPRESS) $(LDFLAGS) $^ -o $@


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

test_valgrind: build
	@for t in $(TEST_EXECUTABLES); do \
		valgrind --leak-resolution=low --quiet \
		--leak-check=full --show-leak-kinds=all --show-possibly-lost=no \
		--suppressions=unittests.supp ./$$t; \
	done

clean:
	@rm -f gtest.a gtest_main.a *.o *.run

