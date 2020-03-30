import os
import sys
from RLTest import Env
from redisgraph import Graph, Node, Edge

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from base import FlowTestsBase

redis_graph = None

GRAPH_ID = "G"

class testRelationPattern(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(GRAPH_ID, redis_con)
        self.populate_graph()

    def populate_graph(self):
        # Construct a graph with the form:
        # (v1)-[:e]->(v2)-[:e]->(v3)
        node_props = ['v1', 'v2', 'v3']

        nodes = []
        for idx, v in enumerate(node_props):
            node = Node(label="L", properties={"val": v})
            nodes.append(node)
            redis_graph.add_node(node)

        edge = Edge(nodes[0], "e", nodes[1])
        redis_graph.add_edge(edge)

        edge = Edge(nodes[1], "e", nodes[2])
        redis_graph.add_edge(edge)

        redis_graph.commit()

    # Test patterns that traverse 1 edge.
    def test01_one_hop_traversals(self):
        # Conditional traversal with label
        query = """MATCH (a)-[:e]->(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        result_a = redis_graph.query(query)

        # Conditional traversal without label
        query = """MATCH (a)-[]->(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        result_b = redis_graph.query(query)

        # Fixed-length 1-hop traversal with label
        query = """MATCH (a)-[:e*1]->(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        result_c = redis_graph.query(query)

        # Fixed-length 1-hop traversal without label
        query = """MATCH (a)-[*1]->(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        result_d = redis_graph.query(query)

        self.env.assertEquals(result_b.result_set, result_a.result_set)
        self.env.assertEquals(result_c.result_set, result_a.result_set)
        self.env.assertEquals(result_d.result_set, result_a.result_set)

    # Test patterns that traverse 2 edges.
    def test02_two_hop_traversals(self):
        # Conditional two-hop traversal without referenced intermediate node
        query = """MATCH (a)-[:e]->()-[:e]->(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        actual_result = redis_graph.query(query)
        expected_result = [['v1', 'v3']]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Fixed-length two-hop traversal (same expected result)
        query = """MATCH (a)-[:e*2]->(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Variable-length traversal with a minimum bound of 2 (same expected result)
        query = """MATCH (a)-[*2..]->(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Conditional two-hop traversal with referenced intermediate node
        query = """MATCH (a)-[:e]->(b)-[:e]->(c) RETURN a.val, b.val, c.val ORDER BY a.val, b.val"""
        actual_result = redis_graph.query(query)
        expected_result = [['v1', 'v2', 'v3']]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Test variable-length patterns
    def test03_var_len_traversals(self):
        # Variable-length traversal with label
        query = """MATCH (a)-[:e*]->(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        actual_result = redis_graph.query(query)
        expected_result = [['v1', 'v2'],
                           ['v1', 'v3'],
                           ['v2', 'v3']]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Variable-length traversal without label (same expected result)
        query = """MATCH (a)-[*]->(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Variable-length traversal with bounds 1..2 (same expected result)
        query = """MATCH (a)-[:e*1..2]->(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Variable-length traversal with bounds 0..1
        # This will return every node and itself, as well as all
        # single-hop edges.
        query = """MATCH (a)-[:e*0..1]->(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        actual_result = redis_graph.query(query)
        expected_result = [['v1', 'v1'],
                           ['v1', 'v2'],
                           ['v2', 'v2'],
                           ['v2', 'v3'],
                           ['v3', 'v3']]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Test variable-length patterns with alternately labeled source
    # and destination nodes, which can cause different execution sequences.
    def test04_variable_length_labeled_nodes(self):
        # Source and edge labeled variable-length traversal
        query = """MATCH (a:L)-[:e*]->(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        actual_result = redis_graph.query(query)
        expected_result = [['v1', 'v2'],
                           ['v1', 'v3'],
                           ['v2', 'v3']]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Destination and edge labeled variable-length traversal (same expected result)
        query = """MATCH (a)-[:e*]->(b:L) RETURN a.val, b.val ORDER BY a.val, b.val"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Source labeled variable-length traversal (same expected result)
        query = """MATCH (a:L)-[*]->(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Destination labeled variable-length traversal (same expected result)
        query = """MATCH (a)-[*]->(b:L) RETURN a.val, b.val ORDER BY a.val, b.val"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Test traversals over explicit relationship types
    def test05_relation_types(self):
        # Add two nodes and two edges of a new type.
        # The new form of the graph will be:
        # (v1)-[:e]->(v2)-[:e]->(v3)-[:q]->(v4)-[:q]->(v5)
        query = """MATCH (n {val: 'v3'}) CREATE (n)-[:q]->(:L {val: 'v4'})-[:q]->(:L {val: 'v5'})"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.nodes_created, 2)
        self.env.assertEquals(actual_result.relationships_created, 2)

        # Verify the graph structure
        query = """MATCH (a)-[e]->(b) RETURN a.val, b.val, TYPE(e) ORDER BY TYPE(e), a.val, b.val"""
        actual_result = redis_graph.query(query)
        expected_result = [['v1', 'v2', 'e'],
                           ['v2', 'v3', 'e'],
                           ['v3', 'v4', 'q'],
                           ['v4', 'v5', 'q']]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Verify conditional traversals with explicit relation types
        query = """MATCH (a)-[:e]->(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        actual_result = redis_graph.query(query)
        expected_result = [['v1', 'v2'],
                           ['v2', 'v3']]
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """MATCH (a)-[:q]->(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        actual_result = redis_graph.query(query)
        expected_result = [['v3', 'v4'],
                           ['v4', 'v5']]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Verify conditional traversals with multiple explicit relation types
        query = """MATCH (a)-[e:e|:q]->(b) RETURN a.val, b.val, TYPE(e) ORDER BY TYPE(e), a.val, b.val"""
        actual_result = redis_graph.query(query)
        expected_result = [['v1', 'v2', 'e'],
                           ['v2', 'v3', 'e'],
                           ['v3', 'v4', 'q'],
                           ['v4', 'v5', 'q']]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Verify variable-length traversals with explicit relation types
        query = """MATCH (a)-[:e*]->(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        actual_result = redis_graph.query(query)
        expected_result = [['v1', 'v2'],
                           ['v1', 'v3'],
                           ['v2', 'v3']]
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """MATCH (a)-[:q*]->(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        actual_result = redis_graph.query(query)
        expected_result = [['v3', 'v4'],
                           ['v3', 'v5'],
                           ['v4', 'v5']]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Verify variable-length traversals with multiple explicit relation types
        query = """MATCH (a)-[:e|:q*]->(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        actual_result = redis_graph.query(query)
        expected_result = [['v1', 'v2'],
                           ['v1', 'v3'],
                           ['v1', 'v4'],
                           ['v1', 'v5'],
                           ['v2', 'v3'],
                           ['v2', 'v4'],
                           ['v2', 'v5'],
                           ['v3', 'v4'],
                           ['v3', 'v5'],
                           ['v4', 'v5']]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Test traversals over transposed edge matrices.
    def test06_transposed_traversals(self):
        # The intermediate node 'b' will be used to form the scan operation because it is filtered.
        # As such, one of the traversals must be transposed.
        query = """MATCH (a)-[e]->(b {val:'v3'})-[]->(c:L) RETURN COUNT(e)"""
        plan = redis_graph.execution_plan(query)

        # Verify that the execution plan contains two traversals following opposing edge directions.
        self.env.assertIn("<-", plan)
        self.env.assertIn("->", plan)

        # Verify results.
        actual_result = redis_graph.query(query)
        expected_result = [[1]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    def test07_transposed_multi_hop(self):
        redis_con = self.env.getConnection()
        g = Graph("tran_multi_hop", redis_con)

        # (a)-[R]->(b)-[R]->(c)<-[R]-(d)<-[R]-(e)
        a = Node(properties={"val": 'a'})
        b = Node(properties={"val": 'b'})
        c = Node(properties={"val": 'c'})
        d = Node(properties={"val": 'd'})
        e = Node(properties={"val": 'e'})
        
        g.add_node(a)
        g.add_node(b)
        g.add_node(c)
        g.add_node(d)
        g.add_node(e)

        ab = Edge(a, "R", b)
        bc = Edge(b, "R", c)
        ed = Edge(e, "R", d)
        dc = Edge(d, "R", c)

        g.add_edge(ab)
        g.add_edge(bc)
        g.add_edge(ed)
        g.add_edge(dc)

        g.flush()

        q = """MATCH (a)-[*2]->(b)<-[*2]-(c) RETURN a.val, b.val, c.val ORDER BY a.val, b.val, c.val"""
        actual_result = g.query(q)
        expected_result = [['a', 'c', 'a'], ['a', 'c', 'e'], ['e', 'c', 'a'], ['e', 'c', 'e']]
        self.env.assertEquals(actual_result.result_set, expected_result)
