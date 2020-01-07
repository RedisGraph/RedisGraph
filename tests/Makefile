
MAKEFLAGS += --no-builtin-rules

.PHONY: test unit flow tck test_valgrind clean

test: tck

unit:
	### unit tests
	@$(MAKE) -C unit all

flow:
	### flow tests
	@$(MAKE) -C flow

tck:
	### Cypher Technology Compatibility Kit (TCK)
	@$(MAKE) -C tck

clean:
	@find . -name '*.[oad]' -type f -delete
	@find . -name '*.run' -type f -delete
	@find . -name '*.pyc' -type f -delete
