.PHONY: test

DOCKER_VG=0
ifeq ($(VALGRIND),)
ifeq ($(OS),macos)
DOCKER_VG=1
endif
endif

test:
ifeq ($(DOCKER_VG),0)
	# no valgrind
	@python3 -m RLTest --module ../../src/redisgraph.so $(TEST_ARGS)
else
	# valgrind in docker
	@echo running docker to run valgrind tck test on MacOS
	@docker run -ti  mac_os_test_docker bash -c "cd tests/tck; RLTest --module ../../src/redisgraph.so $(TEST_ARGS)"
endif
