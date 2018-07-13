test:
	# unit tests
	$(MAKE) -C unit all

	# flow tests
	python -m unittest discover --verbose
	
	# Cypher Technology Compatibility Kit (TCK)
	$(MAKE) -C tck

clean:
	find . -name '*.[oad]' -type f -delete
