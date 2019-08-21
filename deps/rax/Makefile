ifeq ($(DEBUG), 1)
	DEBUGFLAGS = -g -ggdb -O0
endif
CFLAGS?= -O2 -Wall -W -std=c99
LDFLAGS= -lm

# Uncomment the following two lines for coverage testing
#
# CFLAGS+=-fprofile-arcs -ftest-coverage
# LDFLAGS+=-lgcov

all: rax-test rax-oom-test

rax.o: rax.h
rax-test.o: rax.h
rax-oom-test.o: rax.h

rax-test: rax-test.o rax.o rc4rand.o crc16.o
	$(CC) -o $@ $^ $(LDFLAGS) $(DEBUGFLAGS)

rax-oom-test: rax-oom-test.o rax.o
	$(CC) -o $@ $^ $(LDFLAGS) $(DEBUGFLAGS)

.c.o:
	$(CC) -c $(CFLAGS) $(DEBUGFLAGS) $<

clean:
	rm -f rax-test rax-oom-test *.gcda *.gcov *.gcno *.o
