import os
import sys
from RLTest import Env
from redisgraph import Graph, Node, Edge

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from base import FlowTestsBase

graph = None

GRAPH_ID = "AlgebraicExpressionOrder"

class testAlgebraicExpressionOrder(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global graph
        redis_con = self.env.getConnection()
        graph = Graph(GRAPH_ID, redis_con)
        self.populate_graph()

    def populate_graph(self):
        # Construct a graph with the form:
        # (a:A)-[:E]->(b:B), (c:C)-[:E]->(b)
        a = Node(label="A", properties={"v": 1})
        graph.add_node(a)

        b = Node(label="B", properties={"v": 2})
        graph.add_node(b)

        c = Node(label="C", properties={"v": 3})
        graph.add_node(c)

        edge = Edge(a, "E", b)
        graph.add_edge(edge)

        edge = Edge(c, "E", b)
        graph.add_edge(edge)

        graph.commit()

    # Test differing patterns with the same destination node.
    def test01_same_destination_permutations(self):
        # Each query should return the same two records.
        expected_result = [[1, 2],
                           [3, 2]]

        # Neither the source nor the destination is labeled, perform an AllNodeScan from the source node.
        query = """MATCH (a)-[:E]->(b) RETURN a.v, b.v ORDER BY a.v, b.v"""
        plan = graph.execution_plan(query)
        self.env.assertIn("All Node Scan | (a)", plan)
        result = graph.query(query)
        self.env.assertEquals(result.result_set, expected_result)

        # Destination is labeled, perform a LabelScan from the destination node.
        query = """MATCH (a)-[:E]->(b:B) RETURN a.v, b.v ORDER BY a.v, b.v"""
        plan = graph.execution_plan(query)
        self.env.assertIn("Node By Label Scan | (b:B)", plan)
        result = graph.query(query)
        self.env.assertEquals(result.result_set, expected_result)

        # Destination is filtered, perform an AllNodeScan from the destination node.
        query = """MATCH (a)-[:E]->(b) WHERE b.v = 2 RETURN a.v, b.v ORDER BY a.v, b.v"""
        plan = graph.execution_plan(query)
        self.env.assertIn("All Node Scan | (b)", plan)
        result = graph.query(query)
        self.env.assertEquals(result.result_set, expected_result)

        # Destination is labeled but source is filtered, perform an AllNodeScan from the source node.
        query = """MATCH (a)-[:E]->(b:B) WHERE a.v = 1 OR a.v = 3 RETURN a.v, b.v ORDER BY a.v, b.v"""
        plan = graph.execution_plan(query)
        self.env.assertIn("All Node Scan | (a)", plan)
        result = graph.query(query)
        self.env.assertEquals(result.result_set, expected_result)

        # The subsequent queries will only return one record.
        expected_result = [[3, 2]]
        # Both are labeled and source is filtered, perform a LabelScan from the source node.
        query = """MATCH (a:C)-[:E]->(b:B) WHERE a.v = 3 RETURN a.v, b.v ORDER BY a.v, b.v"""
        plan = graph.execution_plan(query)
        self.env.assertIn("Node By Label Scan | (a:C)", plan)
        result = graph.query(query)
        self.env.assertEquals(result.result_set, expected_result)

        # Both are labeled and dest is filtered, perform a LabelScan from the dest node.
        query = """MATCH (a:C)-[:E]->(b:B) WHERE b.v = 2 RETURN a.v, b.v ORDER BY a.v, b.v"""
        plan = graph.execution_plan(query)
        self.env.assertIn("Node By Label Scan | (b:B)", plan)
        result = graph.query(query)
        self.env.assertEquals(result.result_set, expected_result)
