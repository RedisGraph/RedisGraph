
MAKEFLAGS += --no-builtin-rules

.PHONY: test unit flow tck graph500 test_valgrind clean

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

graph500:
	@$(MAKE) -C benchmark_docker graph500

clean:
	@find . -name '*.[oad]' -type f -delete
	@find . -name '*.run' -type f -delete
	@find . -name '*.pyc' -type f -delete
