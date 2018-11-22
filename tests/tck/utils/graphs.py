import redis
import os
import sys

from disposableredis import DisposableRedis
from redisgraph import Graph, Node, Edge

r = None
graph_name = "G"
redis_graph = None

def redis():
    print os.path.dirname(os.path.abspath(__file__)) + '/../../../src/redisgraph.so'
    return DisposableRedis(path='/usr/local/bin/redis-server', loadmodule=os.path.dirname(os.path.abspath(__file__)) + '/../../../src/redisgraph.so')

def _brand_new_redis():
    global r
    if r is not None:
        r.stop()

    r = redis()
    r.start()
    return r.client()

    # return redis.Redis()

def empty_graph():
    global redis_graph
    
    redis_con = _brand_new_redis()
    redis_graph = Graph("G", redis_con)

    # Create a graph with a single node.
    redis_graph.add_node(Node())
    redis_graph.commit()

    # Delete node to have an empty graph.
    redis_graph.query("MATCH (n) DELETE n")

def any_graph():
    return empty_graph()

def query(q):
    return redis_graph.query(q)
