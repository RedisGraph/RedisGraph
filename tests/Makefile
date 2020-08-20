
MAKEFLAGS += --no-builtin-rules

.PHONY: test unit flow tck memcheck clean

TEST_ARGS+=--clear-logs

# check verbose flag
ifneq ($(V),)
	TEST_ARGS += --verbose
endif

# check environment flag
ifneq ($(ENV),)
	TEST_ARGS += --env $(ENV)
endif

# If the Valgrind flag is set, check for memory access errors.
ifneq ($(VALGRIND),)
	TEST_ARGS += --vg --vg-no-leakcheck --vg-verbose --vg-suppressions=$(shell pwd)/../src/valgrind.supp
endif

# Set the RLTest arguments to check for memory leaks.
MEMCHECK_ARGS = $(TEST_ARGS) --vg --vg-no-fail-on-errors --vg-suppressions=$(shell pwd)/../src/valgrind.supp

test: unit flow tck

unit:
	### unit tests
	@$(MAKE) -C unit all

flow:
	### flow tests
	@$(MAKE) -C flow TEST_ARGS="$(TEST_ARGS)"

tck:
	### Cypher Technology Compatibility Kit (TCK)
	@$(MAKE) -C tck TEST_ARGS="$(TEST_ARGS)"

memcheck: export RS_GLOBAL_DTORS = 1
memcheck:
	@$(MAKE) -C flow TEST_ARGS="$(MEMCHECK_ARGS)"
	@$(MAKE) -C tck TEST_ARGS="$(MEMCHECK_ARGS)"
	./memcheck.sh

clean:
	@find . -name '*.[oad]' -type f -delete
	@find . -name '*.run' -type f -delete
	@find . -name '*.pyc' -type f -delete
