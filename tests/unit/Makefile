# Adapted from the sample Makefile provided with GoogleTest

# Points to the root of Google Test, relative to where this file is.
GTEST_DIR = ../googletest

# Flags passed to the preprocessor.
# Set Google Test's header directory as a system directory, such that
# the compiler doesn't generate warnings in Google Test headers.
CPPFLAGS += -isystem $(GTEST_DIR)/include

# Flags passed to the C++ compiler.
CXXFLAGS += -g -Wall -Wextra -pthread
CXX_SUPPRESS = -Wno-sign-compare -Wno-unused-variable

# Google Test headers - do not modify
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h

# RedisGraph flags and libraries

# TODO - unused right now (perhaps always?)
# CFLAGS:=$(CFLAGS)

LDFLAGS :=  -lc -lm -ldl -lpthread
CC_OBJECTS:=$(CC_OBJECTS)
LIBRMUTIL=../../src/rmutil/librmutil.a
LIBTRIEMAP=../../src/util/triemap/libtriemap.a
LIBGRAPHBLAS=../../deps/GraphBLAS/build/libgraphblas.a

LIBS=$(LIBTRIEMAP) $(LIBRMUTIL) $(LIBGRAPHBLAS)
DEPS=$(CC_OBJECTS) $(LIBS)

# Test compilation targets
TESTS = test_value test_graph

run_tests: $(TESTS)
	@(sh -c ./test_value)
	@(sh -c ./test_graph)

all : $(TESTS) run_tests

clean :
	rm -f $(TESTS) gtest.a gtest_main.a *.o

# Google Test compiler targets
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

gtest-all.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest-all.cc

gtest_main.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest_main.cc

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) $@ $^


# RedisGraph compiler targets
test_value: test_value.o gtest_main.a $(DEPS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $^ -o $@

test_graph: test_graph.o gtest_main.a $(DEPS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $^ -o $@

