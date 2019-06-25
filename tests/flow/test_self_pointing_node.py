import os
import sys
import unittest
from redisgraph import Graph, Node, Edge

# import redis
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
from disposableredis import DisposableRedis

from base import FlowTestsBase

redis_graph = None

GRAPH_ID = "G"

def redis():
    return DisposableRedis(loadmodule=os.path.dirname(os.path.abspath(__file__)) + '/../../src/redisgraph.so')

class SelfPointingNode(FlowTestsBase):
    @classmethod
    def setUpClass(cls):
        print "SelfPointingNode"
        global redis_graph
        cls.r = redis()
        cls.r.start()
        redis_con = cls.r.client()
        redis_graph = Graph(GRAPH_ID, redis_con)

        cls.populate_graph()

    @classmethod
    def tearDownClass(cls):
        cls.r.stop()
        # pass

    @classmethod
    def populate_graph(cls):
        # Construct a graph with the form:
        # (v1)-[:e]->(v1)

        node = Node(label="L")
        redis_graph.add_node(node)

        edge = Edge(node, "e", node)
        redis_graph.add_edge(edge)

        redis_graph.commit()

    # Test patterns that traverse 1 edge.
    def test_self_pointing_node(self):
        # Conditional traversal with label
        query = """MATCH (a)-[:e]->(a) RETURN a"""
        result_a = redis_graph.query(query)
        plan_a = redis_graph.execution_plan(query)

        query = """MATCH (a:L)-[:e]->(a) RETURN a"""
        result_b = redis_graph.query(query)
        plan_b = redis_graph.execution_plan(query)

        query = """MATCH (a)-[:e]->(a:L) RETURN a"""
        result_c = redis_graph.query(query)
        plan_c = redis_graph.execution_plan(query)

        query = """MATCH (a)-[]->(a) RETURN a"""
        result_d = redis_graph.query(query)
        plan_d = redis_graph.execution_plan(query)

        query = """MATCH (a:L)-[]->(a) RETURN a"""
        result_e = redis_graph.query(query)
        plan_e = redis_graph.execution_plan(query)

        query = """MATCH (a)-[]->(a:L) RETURN a"""
        result_f = redis_graph.query(query)
        plan_f = redis_graph.execution_plan(query)

        assert(len(result_a.result_set) == 1)
        n = result_a.result_set[0][0]
        assert(n.id == 0)

        assert(result_b.result_set == result_a.result_set)
        assert(result_c.result_set == result_a.result_set)
        assert(result_d.result_set == result_a.result_set)
        assert(result_e.result_set == result_a.result_set)
        assert(result_f.result_set == result_a.result_set)

        assert("Expand Into" in plan_a)
        assert("Expand Into" in plan_b)
        assert("Expand Into" in plan_c)
        assert("Expand Into" in plan_d)
        assert("Expand Into" in plan_e)
        assert("Expand Into" in plan_f)
