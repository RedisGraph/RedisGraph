.PHONY: test

test:
# if host is linux
ifeq ($(OS), Linux)
	# regular tests on linux
	@python -m RLTest --module ../../src/redisgraph.so $(TEST_ARGS)
else
# mac
ifeq ($(VALGRIND),)
	# no valgrind
	@python -m RLTest --module ../../src/redisgraph.so $(TEST_ARGS)
else
	# valgrind in docker
	@echo running docker to run valgrind tck test on MacOS
	@docker run -ti  mac_os_test_docker bash -c "cd tests/tck; RLTest --module ../../src/redisgraph.so $(TEST_ARGS)"
endif
endif
