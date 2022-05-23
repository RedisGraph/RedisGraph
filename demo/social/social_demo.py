import os
import sys
import redis
import argparse
from redis.commands.graph import Graph

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/..')
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

def debug(host, port):
    global redis_con
    global redis_graph
    redis_con = redis.Redis(host=host, port=port)
    redis_graph = Graph(redis_con, social_utils.graph_name)

    print("populate_graph")
    social_utils.populate_graph(redis_con, redis_graph)

    print("run_queries")
    run_queries()

def main(argv):
    global redis_con
    global redis_graph

    parser = argparse.ArgumentParser(description='Social demo.', add_help=False)
    parser.add_argument('-h', '--host', dest='host', help='redis host')
    parser.add_argument('-p', '--port', dest='port', type=int, help='redis port')
    parser.add_argument("--debug", action='store_const', const=True)
    args = parser.parse_args()

    if args.debug is not None:
        debug('127.0.0.1', 6379)
    elif args.host is not None and args.port is not None:
        debug(args.host, args.port)
    else:
        with _redis() as redis_con:
            redis_graph = Graph(redis_con, social_utils.graph_name)
            social_utils.populate_graph(redis_con, redis_graph)
            run_queries()

if __name__ == '__main__':
    main(sys.argv[1:])
