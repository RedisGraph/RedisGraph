
MAKEFLAGS += --no-builtin-rules

ROOT:=$(realpath $(shell pwd)/..)

.PHONY: test unit flow tck memcheck benchmark fuzz clean

TARGET:=$(realpath ../src/redisgraph.so)

ifneq ($(REMOTE),)
BENCHMARK_ARGS=run-remote 
else
BENCHMARK_ARGS=run-local 
endif

BENCHMARK_ARGS += --module_path $(TARGET) --required-module graph
ifneq ($(BENCHMARK),)
BENCHMARK_ARGS += --test $(BENCHMARK)
endif

# check environment flag
ifneq ($(ENV),)
TEST_ARGS += --env $(ENV)
endif

ifeq ($(UNIT),1)
TESTS += unit
else ifeq ($(FLOW),1)
TESTS += flow
else ifeq ($(TCK),1)
TESTS += tck
else
TESTS=unit flow tck
endif

test: $(TESTS)

unit:
	### unit tests
	@$(MAKE) -C unit all

flow:
	### flow tests
	@MODULE=$(TARGET) ./flow/tests.sh

tck:
	### Cypher Technology Compatibility Kit (TCK)
	@GEN=0 AOF=0 TCK=1 MODULE=$(TARGET) ./flow/tests.sh

memcheck:
	@TCK=1 VG=1 VG_ACCESS=1 VG_LEAKS=0 MODULE=$(TARGET) ./flow/tests.sh
	./memcheck.sh

benchmark:
	@cd benchmarks && redisbench-admin $(BENCHMARK_ARGS)

fuzz:
	@$(MAKE) -C fuzz FUZZ_TIMEOUT="$(FUZZ_TIMEOUT)"

clean:
	@find . -name '*.[oad]' -type f -delete
	@find . -name '*.run' -type f -delete
	@find . -name '*.pyc' -type f -delete
