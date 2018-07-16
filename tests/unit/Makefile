# Adapted from the sample Makefile provided with GoogleTest

# Points to the root of Google Test, relative to where this file is.
# Remember to tweak this if you move this file.
GTEST_DIR = ../googletest

# Flags passed to the preprocessor.
# Set Google Test's header directory as a system directory, such that
# the compiler doesn't generate warnings in Google Test headers.
CPPFLAGS += -isystem $(GTEST_DIR)/include

# Flags passed to the C++ compiler.
CXXFLAGS += -g -Wall -Wextra -pthread
CXXSUPPRESS = -Wno-write-strings

# Google Test headers - do not modify
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h

# RedisGraph flags and libraries
CFLAGS:=$(CFLAGS)
CC_OBJECTS:=$(CC_OBJECTS)
LIBRMUTIL=../../src/rmutil/librmutil.a
LIBTRIEMAP=../../src/util/triemap/libtriemap.a
LIBGRAPHBLAS=../../deps/GraphBLAS/build/libgraphblas.a

LIBS=$(LIBTRIEMAP) $(LIBRMUTIL) $(LIBGRAPHBLAS)
DEPS=$(CC_OBJECTS) $(LIBS)

# Tests
TESTS = value_handler

run_tests: $(TESTS)
	@(sh -c ./value_handler)

all : $(TESTS) run_tests

clean :
	rm -f $(TESTS) gtest.a gtest_main.a *.o

# Builds gtest.a and gtest_main.a.

# Usually you shouldn't tweak such internal variables, indicated by a
# trailing _.
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

# For simplicity and to avoid depending on Google Test's
# implementation details, the dependencies specified below are
# conservative and not optimized.  This is fine as Google Test
# compiles fast and for ordinary users its source rarely changes.
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

# A test should link with either gtest.a or gtest_main.a, depending on whether it defines its own main()
# function.

SRCDIR := $(shell pwd)
LDFLAGS :=  -lc -lm -ldl -lpthread

.SUFFIXES: .cc .c .so .xo .o

test_value.o : test_value.c
	$(CC) $(CFLAGS) -c test_value.c $(LDFLAGS)

value_handler.o : value_handler.cc $(GTEST_HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CXXSUPPRESS) -c value_handler.cc $(LDFLAGS)

value_handler: ../../src/value.o test_value.o value_handler.o gtest_main.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CXXSUPPRESS) -lpthread $^ -o $@

