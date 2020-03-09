import os
import sys
from RLTest import Env
from redisgraph import Graph, Node, Edge

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from base import FlowTestsBase

redis_graph = None

GRAPH_ID = "G"

class testSelfPointingNode(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(GRAPH_ID, redis_con)
        self.populate_graph()
   
    def populate_graph(self):
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

        self.env.assertEquals(len(result_a.result_set), 1)
        n = result_a.result_set[0][0]
        self.env.assertEquals(n.id, 0)

        self.env.assertEquals(result_b.result_set, result_a.result_set)
        self.env.assertEquals(result_c.result_set, result_a.result_set)
        self.env.assertEquals(result_d.result_set, result_a.result_set)
        self.env.assertEquals(result_e.result_set, result_a.result_set)
        self.env.assertEquals(result_f.result_set, result_a.result_set)

        self.env.assertIn("Expand Into", plan_a)
        self.env.assertIn("Expand Into", plan_b)
        self.env.assertIn("Expand Into", plan_c)
        self.env.assertIn("Expand Into", plan_d)
        self.env.assertIn("Expand Into", plan_e)
        self.env.assertIn("Expand Into", plan_f)
