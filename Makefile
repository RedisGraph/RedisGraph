.PHONY: all clean package docker docker_push builddocs localdocs deploydocs test test_valgrind

all:
	@$(MAKE) -C ./src all

clean:
	@$(MAKE) -C ./src $@

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

format:
	astyle -Q --options=.astylerc -R "./*.c,*.h"