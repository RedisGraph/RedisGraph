import os
import sys
import unittest
from redisgraph import Graph, Node, Edge

# import redis
from .disposableredis import DisposableRedis

from base import FlowTestsBase

redis_graph = None
people = ["Roi", "Alon", "Ailon", "Boaz", "Tal", "Omri", "Ori"]

def redis():
    return DisposableRedis(loadmodule=os.path.dirname(os.path.abspath(__file__)) + '/../../src/redisgraph.so')

class GraphMultiPatternQueryFlowTest(FlowTestsBase):
    @classmethod
    def setUpClass(cls):
        print "GraphMultiPatternQueryFlowTest"
        global redis_graph
        cls.r = redis()
        cls.r.start()
        redis_con = cls.r.client()
        redis_graph = Graph("G", redis_con)

        # cls.r = redis.Redis()
        # redis_graph = Graph("G", cls.r)

        cls.populate_graph()

    @classmethod
    def tearDownClass(cls):
        cls.r.stop()
        # pass

    @classmethod
    def populate_graph(cls):
        global redis_graph
        
        nodes = {}
         # Create entities        
        for p in people:
            node = Node(label="person", properties={"name": p})
            redis_graph.add_node(node)
            nodes[p] = node

        redis_graph.commit()

    # Connect a single node to all other nodes.
    def test01_connect_node_to_rest(self):
        query = """MATCH(r:person {name:"Roi"}), (f:person) WHERE f.name != r.name CREATE (r)-[:friend]->(f)"""
        actual_result = redis_graph.query(query)
        assert (actual_result.relationships_created == 6)
    
    def test02_verify_connect_node_to_rest(self):
        query = """MATCH(r:person {name:"Roi"})-[]->(f) RETURN count(f)"""
        actual_result = redis_graph.query(query)
        friend_count = int(float(actual_result.result_set[1][0]))
        assert(friend_count == 6)

    def test03_verify_cartesian_product_streams_reset(self):
        # See https://github.com/RedisLabsModules/RedisGraph/issues/249
        # Forevery outgoing edge, we expect len(people) to be matched.
        expected_resultset_size = 6 * len(people)
        queries = ["""MATCH(r:person {name:"Roi"})-[]->(f), (x) RETURN f, x""", """MATCH (x), (r:person {name:"Roi"})-[]->(f) RETURN f, x"""]
        for q in queries:
            actual_result = redis_graph.query(q)
            records_count = len(actual_result.result_set) -1 # Discard header row.
            assert(records_count == expected_resultset_size)

    # Connect every node to every node.
    def test04_create_fully_connected_graph(self):
        query = """MATCH(r:person), (f:person) WHERE f.name != r.name CREATE (r)-[:friend]->(f)"""
        actual_result = redis_graph.query(query)
        assert (actual_result.relationships_created == 42)
    
    def test05_verify_fully_connected_graph(self):
        query = """MATCH(r:person)-[]->(f:person) RETURN count(r)"""
        actual_result = redis_graph.query(query)
        friend_count = int(float(actual_result.result_set[1][0]))
        assert(friend_count == 42)
    
    # Perform a cartesian product of 3 sets.
    def test06_cartesian_product(self):
        query = """MATCH(a), (b), (c) RETURN count(a)"""
        actual_result = redis_graph.query(query)
        friend_count = int(float(actual_result.result_set[1][0]))
        assert(friend_count == 343)

    # Ensure that an error is issued when an alias from one pattern is referenced by another.
    def test07_interdependent_patterns(self):
        query = """MATCH(a)-[]->(b), (b)-[]->(c) RETURN count(b)"""
        try:
            actual_result = redis_graph.query(query)
            assert(False)
        except Exception, e:
            assert("may not be referenced in multiple patterns") in e.message

if __name__ == '__main__':
    unittest.main()
