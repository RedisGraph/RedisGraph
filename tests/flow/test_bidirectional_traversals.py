import os
import sys
from RLTest import Env

from redisgraph import Graph, Node, Edge

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from base import FlowTestsBase

redis_con = None
acyclic_graph = None
graph_with_cycle = None

class testBidirectionalTraversals(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        global redis_con
        redis_con = self.env.getConnection()
        self.populate_acyclic_graph()
        self.populate_cyclic_graph()

    def populate_acyclic_graph(self):
        global acyclic_graph
        acyclic_graph = Graph("G", redis_con)
        # Construct a graph with the form:
        # (v1)-[:E]->(v2)-[:E]->(v3)
        node_props = ['v1', 'v2', 'v3']

        nodes = []
        for idx, v in enumerate(node_props):
            node = Node(label="L", properties={"val": v})
            nodes.append(node)
            acyclic_graph.add_node(node)

        edge = Edge(nodes[0], "E", nodes[1], properties={"val": 0})
        acyclic_graph.add_edge(edge)

        edge = Edge(nodes[1], "E", nodes[2], properties={"val": 1})
        acyclic_graph.add_edge(edge)

        acyclic_graph.commit()

    def populate_cyclic_graph(self):
        global graph_with_cycle
        graph_with_cycle = Graph("H", redis_con)
        # Construct a graph with the form:
        # (v1)-[:E]->(v2)-[:E]->(v3), (v2)-[:E]->(v1)
        node_props = ['v1', 'v2', 'v3']

        nodes = []
        for idx, v in enumerate(node_props):
            node = Node(label="L", properties={"val": v})
            nodes.append(node)
            graph_with_cycle.add_node(node)

        edge = Edge(nodes[0], "E", nodes[1])
        graph_with_cycle.add_edge(edge)

        edge = Edge(nodes[1], "E", nodes[2])
        graph_with_cycle.add_edge(edge)

        # Introduce a cycle between v2 and v1.
        edge = Edge(nodes[1], "E", nodes[0])
        graph_with_cycle.add_edge(edge)

        graph_with_cycle.commit()

    # Test traversals that don't specify an edge direction.
    def test01_bidirectional_traversals(self):
        query = """MATCH (a)-[:E]-(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        actual_result = acyclic_graph.query(query)
        # Each relation should appear twice with the source and destination swapped in the second result.
        expected_result = [['v1', 'v2'],
                           ['v2', 'v1'],
                           ['v2', 'v3'],
                           ['v3', 'v2']]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test undirected traversals with a referenced edge.
        query = """MATCH (a)-[e:E]-(b) RETURN ID(e), a.val, b.val ORDER BY a.val, b.val"""
        actual_result = acyclic_graph.query(query)
        expected_result = [[0, 'v1', 'v2'],
                           [0, 'v2', 'v1'],
                           [1, 'v2', 'v3'],
                           [1, 'v3', 'v2']]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Test 0-hop undirected traversals.
    def test02_bidirectional_zero_hop_traversals(self):
        query = """MATCH (a)-[*0]-(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        actual_result = acyclic_graph.query(query)
        expected_result = [['v1', 'v1'],
                           ['v2', 'v2'],
                           ['v3', 'v3']]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # TODO doesn't work - returns each node with itself as source and destination in adition to expected results.
        # Test combinations of directed and undirected traversals.
        #  query = """MATCH (a)-[:E]->()-[]-(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        #  actual_result = acyclic_graph.query(query)
        #  expected_result = [['v1', 'v3']]
        #  self.env.assertEquals(actual_result.result_set, expected_result)

        # TODO doesn't work for the same reason.
        # Test fixed-length multi-hop undirected traversals.
        #  query = """MATCH (a)-[:E*2]-(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        #  actual_result = acyclic_graph.query(query)
        #  expected_result = [[0, 'v1', 'v3'],
                           #  [0, 'v3', 'v1']]
        #  self.env.assertEquals(actual_result.result_set, expected_result)

    # Test variable-length traversals that don't specify an edge direction.
    def test03_bidirectional_variable_length_traversals(self):
        query = """MATCH (a)-[*]-(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        actual_result = acyclic_graph.query(query)
        # Each combination of distinct node source and destination should appear once.
        expected_result = [['v1', 'v2'],
                           ['v1', 'v3'],
                           ['v2', 'v1'],
                           ['v2', 'v3'],
                           ['v3', 'v1'],
                           ['v3', 'v2']]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Should generate the same results as the previous query.
        query = """MATCH (a)-[*1..2]-(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        actual_result = acyclic_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Test collecting self and all direct neighbors.
    def test04_bidirectional_variable_bounded_length_traversals(self):
        query = """MATCH (a)-[*0..1]-(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        actual_result = acyclic_graph.query(query)
        # Each combination of distinct node source and destination should appear once.
        expected_result = [['v1', 'v1'],
                           ['v1', 'v2'],
                           ['v2', 'v1'],
                           ['v2', 'v2'],
                           ['v2', 'v3'],
                           ['v3', 'v2'],
                           ['v3', 'v3']]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Test bidirectional query on nonexistent edge.
    def test05_bidirectional_variable_length_traversals_over_nonexistent_type(self):
        query = """MATCH (a)-[:NONEXISTENT*]-(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        actual_result = acyclic_graph.query(query)
        expected_result = []
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Test bidirectional query on real edge or nonexistent edge.
    def test06_bidirectional_variable_length_traversals_over_partial_existing_types(self):
        query = """MATCH (a)-[:NONEXISTENT|:E*]-(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        actual_result = acyclic_graph.query(query)
        # Each combination of distinct node source and destination should appear once.
        expected_result = [['v1', 'v2'],
                           ['v1', 'v3'],
                           ['v2', 'v1'],
                           ['v2', 'v3'],
                           ['v3', 'v1'],
                           ['v3', 'v2']]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # TODO returns 16 rows; 18 rows expected.
    # The missing two rows are both `['v2', 'v3']
    # Test bidirectional query on two real edge types.
    #  def test07_bidirectional_variable_length_traversals_over_multiple_existing_types(self):
        #  # Generate new dest->src edges between every current src->dest pair.
        #  query = """MATCH (a {val: 'v1'})-[e]->(b {val: 'v2'}) CREATE (a)-[:CLONE]->(b)"""
        #  actual_result = acyclic_graph.query(query)
        #  self.env.assertEquals(actual_result.relationships_created, 1)

        #  query = """MATCH (a)-[:E|:CLONE*]-(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        #  actual_result = acyclic_graph.query(query)
        #  expected_result = [['v1', 'v1'],
                           #  ['v1', 'v1'],
                           #  ['v1', 'v2'],
                           #  ['v1', 'v2'],
                           #  ['v1', 'v3'],
                           #  ['v1', 'v3'],
                           #  ['v2', 'v1'],
                           #  ['v2', 'v1'],
                           #  ['v2', 'v2'],
                           #  ['v2', 'v2'],
                           #  ['v2', 'v3'],
                           #  ['v2', 'v3'],
                           #  ['v2', 'v3'],
                           #  ['v3', 'v1'],
                           #  ['v3', 'v1'],
                           #  ['v3', 'v2'],
                           #  ['v3', 'v2'],
                           #  ['v3', 'v2']]
        #  self.env.assertEquals(actual_result.result_set, expected_result)

    # Test bidirectional query on two real edge types.
    def test08_bidirectional_variable_bounded_length_traversals_over_multiple_existing_types(self):
        # Generate one new edge between v1 and v2.
        query = """MATCH (a {val: 'v1'})-[e]->(b {val: 'v2'}) CREATE (a)-[:CLONE]->(b)"""
        actual_result = acyclic_graph.query(query)
        self.env.assertEquals(actual_result.relationships_created, 1)

        query = """MATCH (a)-[:E|:CLONE*1..2]-(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        actual_result = acyclic_graph.query(query)
        expected_result = [['v1', 'v1'],
                           ['v1', 'v1'],
                           ['v1', 'v2'],
                           ['v1', 'v2'],
                           ['v1', 'v3'],
                           ['v1', 'v3'],
                           ['v2', 'v1'],
                           ['v2', 'v1'],
                           ['v2', 'v2'],
                           ['v2', 'v2'],
                           ['v2', 'v3'],
                           ['v3', 'v1'],
                           ['v3', 'v1'],
                           ['v3', 'v2']]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Delete cloned edge.
        query = """MATCH ()-[e:CLONE]->() DELETE e"""
        actual_result = acyclic_graph.query(query)
        self.env.assertEquals(actual_result.relationships_deleted, 1)

    # Test traversals that don't specify an edge direction in a graph with a cycle.
    def test09_bidirectional_traversals_with_cycle(self):
        # Test undirected traversals with a referenced edge.
        # TODO The variant query in which the edge is not referenced does not work:
        #  query = """MATCH (a)-[:E]-(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        query = """MATCH (a)-[e:E]-(b) RETURN ID(e) AS id, a.val, b.val ORDER BY id, a.val, b.val"""
        actual_result = graph_with_cycle.query(query)
        # Each relation should appear twice with the source and destination swapped in the second result.
        expected_result = [[0, 'v1', 'v2'],
                           [0, 'v2', 'v1'],
                           [1, 'v2', 'v3'],
                           [1, 'v3', 'v2'],
                           [2, 'v1', 'v2'],
                           [2, 'v2', 'v1']]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Test variable-length traversals that don't specify an edge direction.
    def test10_bidirectional_variable_length_traversals_with_cycle(self):
        # TODO returns 16 rows; 18 rows expected.
        # The missing two rows are both `['v2', 'v3']
        #  query = """MATCH (a)-[*]-(b) RETURN a.val, b.val ORDER BY a.val, b.val"""

        query = """MATCH (a)-[*1..2]-(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        actual_result = graph_with_cycle.query(query)
        # Each src/dest pair (including when the source and dest are the same) is returned twice
        # except for (v2)-[]->(v3), which correctly only occurs once as the missing traversal pattern takes 3 hops.
        expected_result = [['v1', 'v1'],
                           ['v1', 'v1'],
                           ['v1', 'v2'],
                           ['v1', 'v2'],
                           ['v1', 'v3'],
                           ['v1', 'v3'],
                           ['v2', 'v1'],
                           ['v2', 'v1'],
                           ['v2', 'v2'],
                           ['v2', 'v2'],
                           ['v2', 'v3'],
                           ['v3', 'v1'],
                           ['v3', 'v1'],
                           ['v3', 'v2']]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Collect self and all direct neighbors with the pattern (v1)-[]-(v2) repeated.
        query = """MATCH (a)-[*0..1]-(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        actual_result = graph_with_cycle.query(query)
        expected_result = [['v1', 'v1'],
                           ['v1', 'v2'],
                           ['v1', 'v2'],
                           ['v2', 'v1'],
                           ['v2', 'v1'],
                           ['v2', 'v2'],
                           ['v2', 'v3'],
                           ['v3', 'v2'],
                           ['v3', 'v3']]
        self.env.assertEquals(actual_result.result_set, expected_result)

    def test11_bidirectional_multiple_edge_type(self):
        # Construct a simple graph:
        # (a)-[E1]->(b), (c)-[E2]->(d)

        g = Graph("multi_edge_type", redis_con)

        a = Node(properties={'val': 'a'})
        b = Node(properties={'val': 'b'})
        c = Node(properties={'val': 'c'})
        d = Node(properties={'val': 'd'})
        g.add_node(a)
        g.add_node(b)
        g.add_node(c)
        g.add_node(d)

        ab = Edge(a, "E1", b)
        cd = Edge(c, "E2", d)
        g.add_edge(ab)
        g.add_edge(cd)

        g.flush()

        query = """MATCH (a)-[:E1|:E2]-(z) RETURN a.val, z.val ORDER BY a.val, z.val"""
        actual_result = g.query(query)

        expected_result = [['a', 'b'],
                           ['b', 'a'],
                           ['c', 'd'],
                           ['d', 'c']]

        self.env.assertEquals(actual_result.result_set, expected_result)

    # Test bidirectional traversals resolved by an ExpandInto op.
    def test12_bidirectional_expand_into(self):
        query = """MATCH (a), (b) WITH a, b MATCH (a)-[e:E]-(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        actual_result = acyclic_graph.query(query)
        # Each relation should appear twice with the source and destination swapped in the second result.
        expected_result = [['v1', 'v2'],
                           ['v2', 'v1'],
                           ['v2', 'v3'],
                           ['v3', 'v2']]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Verify result against the equivalent conditional traversal.
        query = """MATCH (a)-[:E]-(b) RETURN a.val, b.val ORDER BY a.val, b.val"""
        traverse_result = acyclic_graph.query(query)
        self.env.assertEquals(actual_result.result_set, traverse_result.result_set)

        # Test undirected traversals with a referenced edge.
        query = """MATCH (a), (b) WITH a, b MATCH (a)-[e:E]-(b) RETURN e.val, a.val, b.val ORDER BY e.val, a.val, b.val"""
        actual_result = acyclic_graph.query(query)
        expected_result = [[0, 'v1', 'v2'],
                           [0, 'v2', 'v1'],
                           [1, 'v2', 'v3'],
                           [1, 'v3', 'v2']]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Verify result against the equivalent conditional traversal.
        query = """MATCH (a)-[e:E]-(b) RETURN e.val, a.val, b.val ORDER BY e.val, a.val, b.val"""
        traverse_result = acyclic_graph.query(query)
        self.env.assertEquals(actual_result.result_set, traverse_result.result_set)
