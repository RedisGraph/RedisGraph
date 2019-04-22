
MAKEFLAGS += --no-builtin-rules

.PHONY: test unit flow tck test_valgrind clean

test: flow tck

unit:
	### unit tests
	@$(MAKE) -C unit all

flow:
	### flow tests
ifeq ($(V),)
	@python -m unittest discover -b
else
	@python -m unittest discover --verbose
	# @python -m unittest discover --verbose -p test_social.py
endif

tck:
	### Cypher Technology Compatibility Kit (TCK)
	@$(MAKE) -C tck

test_valgrind:
	@$(MAKE) -C unit test_valgrind

clean:
	@find . -name '*.[oad]' -type f -delete
	@find . -name '*.run' -type f -delete
	@find . -name '*.pyc' -type f -delete
