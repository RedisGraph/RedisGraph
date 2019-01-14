.PHONY: all clean package docker docker_push builddocs localdocs deploydocs test test_valgrind

all:
	@$(MAKE) -C ./src all

clean:
	@$(MAKE) -C ./src $@

clean-parser:
	$(MAKE) -C ./deps/libcypher-parser distclean

clean-graphblas:
	$(MAKE) -C ./deps/GraphBLAS clean

package: all
	@$(MAKE) -C ./src package

docker:
	@docker build . -t redislabs/redisgraph

docker_push: docker
	@docker push redislabs/redisgraph:latest

builddocs:
	@mkdocs build

localdocs: builddocs
	@mkdocs serve

deploydocs: builddocs
	@mkdocs gh-deploy

test:
	@$(MAKE) -C ./src test

test_valgrind:
	@$(MAKE) -C ./src test_valgrind
