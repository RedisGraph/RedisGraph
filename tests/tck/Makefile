.PHONY: test
# find the OS
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')
TEST_ARGS = 
# check verbose flag
ifeq ($(V),)

else
	TEST_ARGS += --verbose
endif

# check environment flag
ifeq ($(ENV),)
	
else
	TEST_ARGS += --env $(ENV)
endif

#check valgrind flag
ifeq ($(VALGRIND),)
	
else
	valgrind += -V --vg-no-leakcheck --vg-verbose
	TEST_ARGS += $(valgrind)
endif

ifeq ($(uname_S),Linux)
	OS = Linux
else
	OS = Mac
endif


test:
ifeq ($(OS), Linux)
	@python -m RLTest --module ../../src/redisgraph.so  $(TEST_ARGS)
else
ifeq ($(valgrind),)
	@python -m RLTest --module ../../src/redisgraph.so  $(TEST_ARGS)
else
	@echo running docker to run valgrind TCK tests on MacOS
	@docker run -ti  mac_os_test_docker bash -c "cd tests/tck/; RLTest --module ../../src/redisgraph.so $(TEST_ARGS)"
endif
endif
