
MAKEFLAGS += --no-builtin-rules

.PHONY: test unit flow tck test_valgrind clean

test: unit flow tck

unit:
	### unit tests
	@$(MAKE) -C unit all

flow:
	### flow tests
	@$(MAKE) -C flow

tck:
	### Cypher Technology Compatibility Kit (TCK)
	@$(MAKE) -C tck

# TODO Temporary solution which assumes that 'redis-server' is on the path and has been compiled with jemalloc profiling enabled.
# It would be preferable to instantiate a server through RLTest.
memcheck:
	JE_MALLOC_CONF="prof:true,prof_leak:true,lg_prof_sample:0,prof_final:true" LD_PRELOAD="$(shell find .. -name "*.so")" redis-server --loadmodule ../src/redisgraph.so &
	@sleep 1
	@redis-cli ping
	@$(MAKE) TEST_ARGS+=--env-reuse flow
	@$(MAKE) TEST_ARGS+=--env-reuse tck
	redis-cli flushall && redis-cli shutdown

clean:
	@find . -name '*.[oad]' -type f -delete
	@find . -name '*.run' -type f -delete
	@find . -name '*.pyc' -type f -delete
