# find the OS
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')

# Compile flags for linux / osx
ifeq ($(uname_S),Linux)
	SHOBJ_CFLAGS ?=  -fno-common -g -ggdb
	SHOBJ_LDFLAGS ?= -shared
else
	SHOBJ_CFLAGS ?= -dynamic -fno-common -g -ggdb
	SHOBJ_LDFLAGS ?= -bundle -undefined dynamic_lookup
endif

ifndef RMUTIL_LIBDIR
	RMUTIL_LIBDIR=./rmutil
endif

.SUFFIXES: .c .so .xo .o

all: graph.so

clean:
	rm -rf *.xo *.so *.o

test_filter: test_filter.o filter.o
	$(CC) -Wall -o test_filter filter.o test_filter.o $(LIBS) -L$(RMUTIL_LIBDIR) -lrmutil -lc -O0
	@(sh -c ./test_filter)

test_graph: test_graph.o graph.o filter.o
	$(CC) -Wall -o test_graph graph.o test_graph.o filter.o $(LIBS) -L$(RMUTIL_LIBDIR) -lrmutil -lc -O0
	@(sh -c ./test_graph)

.c.xo:
	$(CC) -I. $(CFLAGS) $(SHOBJ_CFLAGS) -fPIC -c $< -o $@

graph.xo: redismodule.h

graph.so: graph.xo
	$(LD) -o $@ $< $(SHOBJ_LDFLAGS) $(LIBS) -lc
