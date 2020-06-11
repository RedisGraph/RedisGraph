import os
import sys
from RLTest import Env
from redisgraph import Graph, Node, Edge

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from base import FlowTestsBase

GRAPH_ID = "G"
default_graph = None

class testTransposeConfigurations(FlowTestsBase):
    def setUp(self):
        # At the start of each test, instantiate a server with default configurations and build the graph.
        global default_graph
        self.env = Env()
        redis_con = self.env.getConnection()
        default_graph = Graph(GRAPH_ID, redis_con)
        self.populate_graph(default_graph)

    def populate_graph(self, graph):
        # Construct a graph with the form:
        # (v1)-[:E]->(v2)-[:E]->(v3), (v1)-[:E]->(v3)
        node_props = ['v1', 'v2', 'v3']

        nodes = []
        for idx, v in enumerate(node_props):
            node = Node(label="L", properties={"val": v})
            nodes.append(node)
            graph.add_node(node)

        edge = Edge(nodes[0], "E", nodes[1])
        graph.add_edge(edge)

        edge = Edge(nodes[1], "E", nodes[2])
        graph.add_edge(edge)

        edge = Edge(nodes[0], "E", nodes[2])
        graph.add_edge(edge)

        graph.flush()

    # Test patterns that traverse 1 edge.
    def test01_simple_traversal(self):
        # All queries in this test should produce the same result.
        expected_result = [['v1', 'v2'],
                           ['v1', 'v3'],
                           ['v2', 'v3']]

        # Conditional traversal over standard matrix
        traverse_query = """MATCH (a:L)-[:E]->(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        # Conditional traversal over transposed matrix
        transpose_traverse_query = """MATCH (a)-[:E]->(b:L) RETURN a.val, b.val ORDER BY a.val, b.val"""

        default_traverse_result = default_graph.query(traverse_query)
        default_transpose_traverse_result = default_graph.query(transpose_traverse_query)

        # Validate the default graph's output
        self.env.assertEquals(default_traverse_result.result_set, expected_result)
        self.env.assertEquals(default_transpose_traverse_result.result_set, expected_result)

        # Flush and tear down the default environment
        self.env.flush()
        self.env.stop()

        # Instantiate a new server without transposed matrices
        configured_env = Env(moduleArgs="MAINTAIN_TRANSPOSED_MATRICES no")
        configured_graph = Graph(GRAPH_ID, configured_env.getConnection())
        # Repopulate the graph
        self.populate_graph(configured_graph)

        configured_traverse_result = configured_graph.query(traverse_query)
        configured_transpose_traverse_result = configured_graph.query(transpose_traverse_query)

        # Validate the configured graph's output
        configured_env.assertEquals(configured_traverse_result.result_set, expected_result)
        configured_env.assertEquals(configured_transpose_traverse_result.result_set, expected_result)

        # Validate that the output is the same with both configurations
        configured_env.assertEquals(default_traverse_result.result_set, configured_traverse_result.result_set)
        configured_env.assertEquals(default_transpose_traverse_result.result_set, configured_transpose_traverse_result.result_set)

        # Flush and tear down the new environment
        configured_env.flush()
        configured_env.stop()

    # Test patterns that traverse both directions.
    def test02_bidirectional_traversal(self):
        # Bidirectional traversal over matrix - E + T(E)
        query = """MATCH (a:L)-[:E]-(b:L) RETURN a.val, b.val ORDER BY a.val, b.val"""
        expected_result = [['v1', 'v2'],
                           ['v1', 'v3'],
                           ['v2', 'v1'],
                           ['v2', 'v3'],
                           ['v3', 'v1'],
                           ['v3', 'v2']]

        default_result = default_graph.query(query)

        # Validate the default graph's output
        self.env.assertEquals(default_result.result_set, expected_result)

        # Flush and tear down the default environment
        self.env.flush()
        self.env.stop()

        # Instantiate a new server without transposed matrices
        configured_env = Env(moduleArgs="MAINTAIN_TRANSPOSED_MATRICES no")
        configured_graph = Graph(GRAPH_ID, configured_env.getConnection())
        # Repopulate the graph
        self.populate_graph(configured_graph)

        configured_result = configured_graph.query(query)
        # Validate the configured graph's output
        configured_env.assertEquals(configured_result.result_set, expected_result)

        # Validate that the output is the same with both configurations
        configured_env.assertEquals(configured_result.result_set, default_result.result_set)

        # Flush and tear down the new environment
        configured_env.flush()
        configured_env.stop()

    # Test patterns that perform repeated multiplications
    def test03_bidirectional_transposed_traversal(self):
        # Two-hop traversal over transposed matrix
        query = """MATCH (a)-[:E*2]->(b:L {val: 'v3'}) RETURN a.val, b.val ORDER BY a.val, b.val"""
        expected_result = [['v1', 'v3']]

        default_result = default_graph.query(query)

        # Validate the default graph's output
        self.env.assertEquals(default_result.result_set, expected_result)

        # Flush and tear down the default environment
        self.env.flush()
        self.env.stop()

        # Instantiate a new server without transposed matrices
        configured_env = Env(moduleArgs="MAINTAIN_TRANSPOSED_MATRICES no")
        configured_graph = Graph(GRAPH_ID, configured_env.getConnection())
        # Repopulate the graph
        self.populate_graph(configured_graph)

        configured_result = configured_graph.query(query)
        # Validate the configured graph's output
        configured_env.assertEquals(configured_result.result_set, expected_result)

        # Validate that the output is the same with both configurations
        configured_env.assertEquals(configured_result.result_set, default_result.result_set)
