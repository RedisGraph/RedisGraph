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
        query = """MATCH(r:person {name:"Roi"}), (f:person) WHERE f.name != r.name CREATE (r)-[:friend]->(f) RETURN count(f)"""
        actual_result = redis_graph.query(query)
        friend_count = int(float(actual_result.result_set[1][0]))
        assert(friend_count == 6)
        assert (actual_result.relationships_created == 6)

    def test02_verify_cartesian_product_streams_reset(self):
        # See https://github.com/RedisLabsModules/RedisGraph/issues/249
        # Forevery outgoing edge, we expect len(people) to be matched.
        expected_resultset_size = 6 * len(people)
        queries = ["""MATCH (r:person {name:"Roi"})-[]->(f), (x) RETURN f, x""",
                   """MATCH (x), (r:person {name:"Roi"})-[]->(f) RETURN f, x""",
                   """MATCH (r:person {name:"Roi"})-[]->(f) MATCH (x) RETURN f, x""",
                   """MATCH (x) MATCH (r:person {name:"Roi"})-[]->(f) RETURN f, x"""]
        for q in queries:
            actual_result = redis_graph.query(q)
            records_count = len(actual_result.result_set) -1 # Discard header row.
            assert(records_count == expected_resultset_size)

    # Connect every node to every node.
    def test03_create_fully_connected_graph(self):
        query = """MATCH(a:person), (b:person) WHERE a.name != b.name CREATE (a)-[f:friend]->(b) RETURN count(f)"""
        actual_result = redis_graph.query(query)
        friend_count = int(float(actual_result.result_set[1][0]))
        assert(friend_count == 42)
        assert (actual_result.relationships_created == 42)
    
    # Perform a cartesian product of 3 sets.
    def test04_cartesian_product(self):
        queries = ["""MATCH (a), (b), (c) RETURN count(a)""",
                   """MATCH (a) MATCH (b), (c) RETURN count(a)""",
                   """MATCH (a), (b) MATCH (c) RETURN count(a)""",
                   """MATCH (a) MATCH (b) MATCH (c) RETURN count(a)"""]

        for q in queries:
            actual_result = redis_graph.query(q)
            friend_count = int(float(actual_result.result_set[1][0]))
            assert(friend_count == 343)

    # Ensure that an error is issued when an alias from one pattern is referenced by another.
    def test05_interdependent_patterns(self):
        queries = ["""MATCH (a)-[]->(b), (b)-[]->(c) RETURN count(b)""",
                   """MATCH (a)-[]->(b) MATCH (b)-[]->(c) RETURN count(b)"""]
        for q in queries:
            try:
                redis_graph.query(q)
                assert(False)
            except Exception, e:
                assert("may not be referenced in multiple patterns") in e.message

    def test06_multiple_create_clauses(self):
        queries = ["""CREATE (:a {v:1}), (:b {v:2, z:3}), (:c), (:a)-[:r0 {k:9}]->(:b), (:c)-[:r1]->(:d)""",
                   """CREATE (:a {v:1}) CREATE (:b {v:2, z:3}) CREATE (:c) CREATE (:a)-[:r0 {k:9}]->(:b) CREATE (:c)-[:r1]->(:d)""",
                   """CREATE (:a {v:1}), (:b {v:2, z:3}) CREATE (:c), (:a)-[:r0 {k:9}]->(:b) CREATE (:c)-[:r1]->(:d)"""]
        for q in queries:
            actual_result = redis_graph.query(q)
            assert (actual_result.relationships_created == 2)
            assert (actual_result.properties_set == 4)
            assert (actual_result.nodes_created == 7)

if __name__ == '__main__':
    unittest.main()
