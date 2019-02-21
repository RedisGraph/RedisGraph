import os
import sys
import unittest
from redisgraph import Graph, Node, Edge

# import redis
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
from disposableredis import DisposableRedis

from base import FlowTestsBase

redis_graph = None

def redis():
    return DisposableRedis(loadmodule=os.path.dirname(os.path.abspath(__file__)) + '/../../src/redisgraph.so')

class SchemaFlowTest(FlowTestsBase):
    @classmethod
    def setUpClass(cls):
        print "SchemaFlowTest"
        global redis_graph
        cls.r = redis()
        cls.r.start()
        redis_con = cls.r.client()
        redis_graph = Graph("G", redis_con)

        # cls.r = redis.Redis()
        # redis_graph = Graph("G", cls.r)

    @classmethod
    def tearDownClass(cls):
        cls.r.stop()
        # pass

    # Perform 4 schema updates with a node creation
    def test01_single_node_with_label(self):
        global redis_graph
        query = """CREATE (:foo{p1:1,p1:1,p12:3,p14:4})"""
        result = redis_graph.query(query)
        assert(result.labels_added == 1)
        assert(result.nodes_created == 1)
        # Since 'p1' is specified twice, properties_set may respond with 3 in the future
        assert(result.properties_set == 4)
        query = """MATCH (a:foo) RETURN a"""
        result = redis_graph.query(query)
        assert(result.result_set == [['a.p1', 'a.p14', 'a.p12'], [1, 4, 3]])

if __name__ == '__main__':
    unittest.main()
