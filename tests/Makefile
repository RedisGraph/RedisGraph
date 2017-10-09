test:
	# unit tests
	$(MAKE) -C unit all

	# flow tests
	python -m unittest discover

clean:
	find . -name '*.[oad]' -type f -delete
