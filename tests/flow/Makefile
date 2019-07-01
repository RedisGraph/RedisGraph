.PHONY: test
# find the OS
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')
TEST_ARGS = 
# check verbose flag
ifneq ($(V),)
	TEST_ARGS += --verbose
endif

# check environment flag
ifeq ($(ENV),)
	
else
	TEST_ARGS += --env $(ENV)
endif

#check valgrind flag
ifneq ($(VALGRIND),)
	valgrind += -V --vg-no-leakcheck --vg-verbose
	TEST_ARGS += $(valgrind)
endif

# check host os 
ifeq ($(uname_S),Linux)
	OS = Linux
else
	OS = Mac
endif

test:
# if host is linux
ifeq ($(OS), Linux)
	# regular tests on linux
	@python -m RLTest --module ../../src/redisgraph.so  $(TEST_ARGS)
else
# mac
ifeq ($(VALGRIND),)
	# no valgrind
	@python -m RLTest --module ../../src/redisgraph.so  $(TEST_ARGS)
else
	# valgrind in docker
	@echo running docker to run valgrind flow test on MacOS
	@docker run -ti  mac_os_test_docker bash -c "cd tests/flow/; RLTest --module ../../src/redisgraph.so $(TEST_ARGS)"
endif
endif
