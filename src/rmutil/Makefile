# set environment variable RM_INCLUDE_DIR to the location of redismodule.h
ifndef RM_INCLUDE_DIR
	RM_INCLUDE_DIR=../
endif

CFLAGS = -g -fPIC -O3 -std=gnu99 -I$(RM_INCLUDE_DIR) -Wall -Wno-unused-function

OBJS=vector.o

all: librmutil.a

clean:
	rm -rf *.o *.a

librmutil.a: $(OBJS)
	ar rcs $@ $^

