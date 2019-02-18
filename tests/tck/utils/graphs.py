import redis
import os
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), '../..'))
from disposableredis import DisposableRedis

from redisgraph import Graph, Node, Edge

r = None
graph_name = "G"
redis_graph = None

def redis():
    graph_so = os.path.dirname(os.path.abspath(__file__)) + '/../../../src/redisgraph.so'
    print graph_so
    return DisposableRedis(loadmodule=graph_so)

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

def teardown():
    global r
    if r is not None:
        r.stop()
