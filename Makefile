MAKEFLAGS += --no-print-directory

.PHONY: all parser clean package docker docker_push docker_alpine builddocs localdocs deploydocs test benchmark test_valgrind fuzz help

define HELP
make all              # Build everything
  DEBUG=1               # Build for debugging
  COV=1                 # Build for coverage analysis (implies DEBUG=1)
make clean            # Clean build artifacts
  ALL=1                 # Completely remove
make test             # Run tests
  COV=1                 # Perform coverage analysis
  UNIT=1                # Run unit tests
  FLOW=1                # Run flow tests (Python)
  TCK=1                 # Run TCK framework tests
  RLEC=1                # Run tests on RLEC
make memcheck         # Run tests with Valgrind
make benchmark        # Run benchmarks
make fuzz             # Run fuzz tester

make package          # Build RAMP packages
make cov-upload       # Upload coverage data to codecov.io

make format           # Apply source code formatting

endef

all:
	@$(MAKE) -C src all

clean:
	@$(MAKE) -C src $@

clean-parser:
	$(MAKE) -C deps/libcypher-parser distclean

clean-graphblas:
	$(MAKE) -C deps/GraphBLAS clean

pack package: all
	@$(MAKE) -C src package

docker:
	@$(MAKE) -C build/docker

docker_alpine:
	@$(MAKE) -C build/docker OSNICK=alpine3

docker_push: docker
	@docker push redislabs/redisgraph:latest

builddocs:
	@mkdocs build

localdocs: builddocs
	@mkdocs serve

deploydocs: builddocs
	@mkdocs gh-deploy

test:
	@$(MAKE) -C src test

benchmark:
	@$(MAKE) -C src benchmark

memcheck:
	@$(MAKE) -C src memcheck

cov-upload:
	@$(MAKE) -C src cov-upload

fuzz:
	@$(MAKE) -C src fuzz

format:
	astyle -Q --options=.astylerc -R --ignore-exclude-errors "./*.c,*.h,*.cpp"

ifneq ($(HELP),)
ifneq ($(filter help,$(MAKECMDGOALS)),)
HELPFILE:=$(shell mktemp /tmp/make.help.XXXX)
endif
endif

help:
	$(file >$(HELPFILE),$(HELP))
	@echo
	@cat $(HELPFILE)
	@echo
	@-rm -f $(HELPFILE)
