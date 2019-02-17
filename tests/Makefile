
.PHONY: test unit flow tck test_valgrind clean

test: unit flow tck

# unit tests
unit:
	$(MAKE) -C unit all

# flow tests
flow:
	python -m unittest discover --verbose

# Cypher Technology Compatibility Kit (TCK)
tck:
	$(MAKE) -C tck

test_valgrind:
	$(MAKE) -C unit test_valgrind

clean:
	@find . -name '*.[oad]' -type f -delete
	@find . -name '*.run' -type f -delete
