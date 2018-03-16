import os
import sys
import redis
from redisgraph import Graph

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/../')
from social_queries import queries_info
import social_utils
from utils import execute_query, _redis


redis_con = None
redis_graph = None


def run_queries():
    print("Querying...\n")

    for query_info in queries_info:
        execute_query(redis_graph,
                      query_info.description,
                      query_info.query)


def debug():
    print("debug")
    global redis_con
    global redis_graph
    redis_con = redis.Redis(host='localhost', port=6379)
    redis_graph = Graph(social_utils.graph_name, redis_con)

    print("populate_graph")
    social_utils.populate_graph(redis_con, redis_graph)

    print("run_queries")
    run_queries()


def main(argv):
    global redis_con
    global redis_graph
    if "--debug" in argv:
        debug()
    else:
        with _redis() as redis_con:
            redis_graph = Graph(social_utils.graph_name, redis_con)
            social_utils.populate_graph(redis_con, redis_graph)
            run_queries()


if __name__ == '__main__':
    main(sys.argv[1:])
