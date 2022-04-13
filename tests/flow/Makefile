.PHONY: test

ROOT:=$(realpath ../..)
TARGET=$(ROOT)/src/redisgraph.so)

DOCKER_VG=0
ifeq ($(VALGRIND),)
ifeq ($(OS),macos)
DOCKER_VG=1
endif
endif

test:
ifeq ($(DOCKER_VG),0)
	@MODULE=$(TARGET) ./test.sh
else
	# valgrind in docker
	@echo running docker to run valgrind flow test on MacOS
	@docker run -ti  mac_os_test_docker bash -c "cd tests/flow/; MODULE=../../src/redisgraph.so ./test.sh"
endif
