.PHONY: graph500
TEST_ARGS =  -g graph500_22 -s graph500_22_seed -c 50 -d 1 -p redisgraph -l graph500_22_unique_node -t 5 -i 10 --stdout

graph500:
	@echo running docker graph500 benchmark
	@docker run --net=host filipecosta90/tigergraph_redisgraph:latest  $(TEST_ARGS)
