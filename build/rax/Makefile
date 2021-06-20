
CFLAGS.debug=-g -ggdb
CFLAGS?= -O2 -Wall -W -std=c99
LDFLAGS= -lm

ifeq ($(COV),1)
CFLAGS+=-fprofile-arcs -ftest-coverage
LDFLAGS+=-lgcov
endif

ifeq ($(DEBUG),1)
CFLAGS += $(CFLAGS.debug)
LDFLAGS += $(CFLAGS.debug)
endif

all: rax-test rax-oom-test

rax.o: rax.h
rax-test.o: rax.h
rax-oom-test.o: rax.h

rax-test: rax-test.o rax.o rc4rand.o crc16.o
	$(CC) -o $@ $^ $(LDFLAGS)

rax-oom-test: rax-oom-test.o rax.o
	$(CC) -o $@ $^ $(LDFLAGS)

.c.o:
	$(CC) -c $(CFLAGS) $<

clean:
	rm -f rax-test rax-oom-test *.gcda *.gcov *.gcno *.o
