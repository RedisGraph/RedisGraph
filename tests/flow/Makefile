.PHONY: test

#----------------------------------------------------------------------------------------------
# Test files

TESTDIR:=$(shell pwd -P)
FLOW_TESTS:=$(wildcard $(TESTDIR)/test_*.py)
FLOW_FILES = $(notdir $(FLOW_TESTS))
#----------------------------------------------------------------------------------------------

%.py: FORCE
#if host is linux
ifeq ($(OS), Linux)
# regular tests on linux
	@python3 -m RLTest --module ../../src/redisgraph.so -r --randomize-ports -t $@ $(TEST_ARGS)
else
# mac
ifeq ($(VALGRIND),)
# no valgrind
	@python3 -m RLTest --module ../../src/redisgraph.so -r --randomize-ports -t $@ $(TEST_ARGS)
else
# valgrind in docker
	@echo running docker to run valgrind flow test on MacOS
	@docker run -ti  mac_os_test_docker bash -c "cd tests/flow/; RLTest --module ../../src/redisgraph.so $(TEST_ARGS)"
endif
endif

test: $(FLOW_FILES)

FORCE:

