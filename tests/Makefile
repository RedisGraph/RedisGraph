
MAKEFLAGS += --no-builtin-rules

ROOT?=$(realpath ..)

.PHONY: test unit flow tck memcheck benchmark fuzz clean

# TARGET:=$(ROOT)/src/redisgraph.so

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

ifneq ($(filter memcheck,$(MAKECMDGOALS)),)
FLOW ?= 1
GEN ?= 1
AOF ?= 1
TCK ?= 1
endif

TESTS=
ifneq ($(UNIT),0)
TESTS += unit
endif
ifneq ($(FLOW),0)
TESTS += flow
endif
ifneq ($(TCK),0)
TESTS += tck
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
	@VG=1 VG_ACCESS=1 VG_LEAKS=1 MODULE=$(TARGET) ./flow/tests.sh
	./memcheck.sh

benchmark:
	@cd benchmarks && redisbench-admin $(BENCHMARK_ARGS)

fuzz:
	@$(MAKE) -C fuzz FUZZ_TIMEOUT="$(FUZZ_TIMEOUT)"

clean:
	@find . -name '*.[oad]' -type f -delete
	@find . -name '*.run' -type f -delete
	@find . -name '*.pyc' -type f -delete
