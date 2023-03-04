.PHONY: all parser clean package docker docker_push docker_alpine builddocs localdocs deploydocs test benchmark test_valgrind help \
	upload-release upload-artifacts

define HELPTEXT
make all              # Build everything
  DEBUG=1               # Build for debugging
  COV=1                 # Build for coverage analysis (implies DEBUG=1)
make clean            # Clean build artifacts

make test             # Run tests
  COV=1                  # Perform coverage analysis
  UNIT=1                 # Run unit tests
  FLOW=1                 # Run flow tests (Python)
  TCK=1                  # Run TCK framework tests
make memcheck         # Run tests with Valgrind
make benchmark        # Run benchmarks

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

package: all
	@$(MAKE) -C src package

upload-release:
	@$(MAKE) -C src upload-release

upload-artifacts:
	@$(MAKE) -C src upload-artifacts

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

format:
	astyle -Q --options=.astylerc -R --ignore-exclude-errors "./*.c,*.h,*.cpp"

ifneq ($(HELPTEXT),)
ifneq ($(filter help,$(MAKECMDGOALS)),)
HELPFILE:=$(shell mktemp /tmp/make.help.XXXX)
endif
endif

help:
	$(file >$(HELPFILE),$(HELPTEXT))
	@echo
	@cat $(HELPFILE)
	@echo
	@-rm -f $(HELPFILE)
