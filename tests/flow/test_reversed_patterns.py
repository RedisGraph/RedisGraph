import os
import sys
import unittest
from redisgraph import Graph, Node, Edge

# import redis
from .disposableredis import DisposableRedis

from base import FlowTestsBase

GRAPH_NAME = "G"
redis_con = None
redis_graph = None

def redis():
    return DisposableRedis(loadmodule=os.path.dirname(os.path.abspath(__file__)) + '/../../src/redisgraph.so')

class GraphPersistency(FlowTestsBase):
    @classmethod
    def setUpClass(cls):
        print "GraphPersistency"
        global redis_graph
        global redis_con
        cls.r = redis()
        cls.r.start()
        redis_con = cls.r.client()
        redis_graph = Graph(GRAPH_NAME, redis_con)

        # redis_con = redis.Redis()
        # redis_graph = Graph("G", redis_con)

        cls.populate_graph()

    @classmethod
    def tearDownClass(cls):
        cls.r.stop()
        # pass

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
