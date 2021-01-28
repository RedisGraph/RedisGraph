.PHONY: all clean package docker docker_push docker_alpine builddocs localdocs deploydocs test test-build memcheck

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

docker_alpine:
	@docker build . -f Dockerfile.alpine -t redislabs/redisgraph:alpine

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

test-build:
	@$(MAKE) -C ./src test-build

memcheck:
	@$(MAKE) -C ./src memcheck

format:
	astyle -Q --options=.astylerc -R --ignore-exclude-errors "./*.c,*.h,*.cpp"
