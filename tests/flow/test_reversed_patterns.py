import os
import sys
import redis
import string
import random
import unittest
from base import FlowTestsBase
from redisgraph import Graph, Node, Edge

redis_con = None
dis_redis = None
redis_graph = None
GRAPH_NAME = None

def random_string(size=6, chars=string.ascii_letters):
    return ''.join(random.choice(chars) for _ in range(size))

def get_redis():
    global dis_redis
    conn = redis.Redis()
    try:
        conn.ping()
        # Assuming RedisGraph is loaded.
    except redis.exceptions.ConnectionError:
        from .disposableredis import DisposableRedis
        # Bring up our own redis-server instance.
        dis_redis = DisposableRedis(loadmodule=os.path.dirname(os.path.abspath(__file__)) + '/../../src/redisgraph.so')
        dis_redis.start()
        conn = dis_redis.client()
    return conn



class GraphPersistency(FlowTestsBase):
    @classmethod
    def setUpClass(cls):
        print "GraphPersistency"
        global redis_graph
        global redis_con
        global GRAPH_NAME
        GRAPH_NAME = random_string()

        redis_con = get_redis()
        redis_graph = Graph(GRAPH_NAME, redis_con)
        cls.populate_graph()

    @classmethod
    def tearDownClass(cls):
        if dis_redis is not None:
            dis_redis.stop()

    @classmethod
    def populate_graph(cls):
        global redis_graph
        if not redis_con.exists(GRAPH_NAME):
            # Create entities
            srcNode = Node(label="L", properties={"name": "SRC"})
            destNode = Node(label="L", properties={"name": "DEST"})
            redis_graph.add_node(srcNode)
            redis_graph.add_node(destNode)
            edge = Edge(srcNode, 'E', destNode)
            redis_graph.add_edge(edge)
            redis_graph.commit()

    # Verify that edges are not modified after entity deletion
    def test01_reversed_pattern(self):
        leftToRight = """MATCH (a:L)-[b]->(c:L) RETURN a, TYPE(b), c"""
        rightToLeft = """MATCH (c:L)<-[b]-(a:L) RETURN a, TYPE(b), c"""
        leftToRightResult = redis_graph.query(leftToRight)
        rightToLeftResult = redis_graph.query(rightToLeft)
        assert(leftToRightResult.result_set == rightToLeftResult.result_set)

if __name__ == '__main__':
    unittest.main()
