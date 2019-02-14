import os
import sys
import unittest
from redisgraph import Graph, Node, Edge

# import redis
from .disposableredis import DisposableRedis

from base import FlowTestsBase

redis_graph = None

values = ["str1", "str2", False, True, 5, 10.5]

def redis():
    return DisposableRedis(loadmodule=os.path.dirname(os.path.abspath(__file__)) + '/../../src/redisgraph.so')

class ValueComparisonTest(FlowTestsBase):
    @classmethod
    def setUpClass(cls):
        print "ValueComparisonTest"
        global redis_graph
        cls.r = redis()
        cls.r.start()
        redis_con = cls.r.client()
        redis_graph = Graph("G", redis_con)

        cls.populate_graph()

    @classmethod
    def tearDownClass(cls):
        cls.r.stop()
        # pass

    @classmethod
    def populate_graph(cls):
        redis_graph

        for v in values:
            node = Node(label="value", properties={"val": v})
            redis_graph.add_node(node)

        # Add an additional node with no properties
        redis_graph.add_node(Node(label="value"))

        redis_graph.commit()

    # Verify the ordering of values that can and cannot be directly compared
    def test_orderability(self):
        query = """MATCH (v:value) RETURN v ORDER BY v.val"""
        actual_result = redis_graph.query(query)
        expected = [['str1'],
                    ['str2'],
                    ['false'],
                    ['true'],
                    [5],
                    ['10.5'],
                    [None]]
        assert(actual_result.result_set[1:] == expected)

        # Expect the results to appear in reverse when using descending order
        query = """MATCH (v:value) RETURN v ORDER BY v.val DESC"""
        actual_result = redis_graph.query(query)
        assert(actual_result.result_set[1:] == expected[::-1])

    # From the Cypher specification:
    # "In a mixed set, any numeric value is always considered to be higher than any string value"
    def test_mixed_type_min(self):
        query = """MATCH (v:value) RETURN MIN(v.val)"""
        actual_result = redis_graph.query(query)
        assert(actual_result.result_set[1][0] == 'str1')

    def test_mixed_type_max(self):
        query = """MATCH (v:value) RETURN MAX(v.val)"""
        actual_result = redis_graph.query(query)
        assert(actual_result.result_set[1][0] == '10.5')

if __name__ == '__main__':
    unittest.main()

