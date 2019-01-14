import os
import sys
import redis
import string
import random
import unittest
from base import FlowTestsBase
from redisgraph import Graph, Node, Edge

redis_graph = None
dis_redis = None
redis_con = None
values = ["str1", "str2", False, True, 5, 10.5]

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

class ValueComparisonTest(FlowTestsBase):
    @classmethod
    def setUpClass(cls):
        print "ValueComparisonTest"
        global redis_graph
        global redis_con
        redis_con = get_redis()
        GRAPH_ID = random_string()
        redis_graph = Graph(GRAPH_ID, redis_con)

        cls.populate_graph()

    @classmethod
    def tearDownClass(cls):
        if dis_redis is not None:
            dis_redis.stop()

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

    # Verify that disjoint types pass != filters
    def test_disjoint_comparisons(self):
        # Compare all node pairs under a Cartesian product
        query = """MATCH (v:value), (w:value) WHERE ID(v) != ID(w) AND v.val = w.val RETURN v"""
        actual_result = redis_graph.query(query)
        # No nodes have the same property, so there should be 0 equal results
        expected_result_count = 0
        assert(len(actual_result.result_set[1:]) == expected_result_count)

        query = """MATCH (v:value), (w:value) WHERE ID(v) != ID(w) AND v.val != w.val RETURN v"""
        actual_result = redis_graph.query(query)
        # Every comparison should produce an inequal result
        node_count = len(redis_graph.nodes)
        expected_result_count = node_count * (node_count - 1)
        assert(len(actual_result.result_set[1:]) == expected_result_count)


if __name__ == '__main__':
    unittest.main()

