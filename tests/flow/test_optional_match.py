import os
import sys
from RLTest import Env
from redisgraph import Graph, Node, Edge
from base import FlowTestsBase

redis_graph = None
nodes = {}

class testOptionalFlow(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph("optional_match", redis_con)
        self.populate_graph()

    def populate_graph(self):
        global nodes
        # Construct a graph with the form:
        # (v1)-[:E1]->(v2)-[:E2]->(v3), (v4)
        node_props = ['v1', 'v2', 'v3', 'v4']

        for idx, v in enumerate(node_props):
            node = Node(label="L", properties={"v": v})
            nodes[v] = node
            redis_graph.add_node(node)

        edge = Edge(nodes['v1'], "E1", nodes['v2'])
        redis_graph.add_edge(edge)

        edge = Edge(nodes['v2'], "E2", nodes['v3'])
        redis_graph.add_edge(edge)

        redis_graph.flush()

    # Optional MATCH clause that does not interact with the mandatory MATCH.
    def test01_disjoint_optional(self):
        global redis_graph
        query = """MATCH (a {v: 'v1'}) OPTIONAL MATCH (b) RETURN a.v, b.v ORDER BY a.v, b.v"""
        actual_result = redis_graph.query(query)
        expected_result = [['v1', 'v1'],
                           ['v1', 'v2'],
                           ['v1', 'v3'],
                           ['v1', 'v4']]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Optional MATCH clause that extends the mandatory MATCH pattern and has matches for all results.
    def test02_optional_traverse(self):
        global redis_graph
        query = """MATCH (a) WHERE a.v IN ['v1', 'v2'] OPTIONAL MATCH (a)-[]->(b) RETURN a.v, b.v ORDER BY a.v, b.v"""
        actual_result = redis_graph.query(query)
        expected_result = [['v1', 'v2'],
                           ['v2', 'v3']]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Optional MATCH clause that extends the mandatory MATCH pattern and has null results.
    def test03_optional_traverse_with_nulls(self):
        global redis_graph
        query = """MATCH (a) OPTIONAL MATCH (a)-[]->(b) RETURN a.v, b.v ORDER BY a.v, b.v"""
        actual_result = redis_graph.query(query)
        # (v3) and (v4) have no outgoing edges.
        expected_result = [['v1', 'v2'],
                           ['v2', 'v3'],
                           ['v3', None],
                           ['v4', None]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Optional MATCH clause that extends the mandatory MATCH pattern and has a WHERE clause.
    def test04_optional_traverse_with_predicate(self):
        global redis_graph
        query = """MATCH (a) OPTIONAL MATCH (a)-[]->(b) WHERE b.v = 'v2' RETURN a.v, b.v ORDER BY a.v, b.v"""
        actual_result = redis_graph.query(query)
        # only (v1) has an outgoing edge to (v2).
        expected_result = [['v1', 'v2'],
                           ['v2', None],
                           ['v3', None],
                           ['v4', None]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Optional MATCH clause with endpoints resolved by the mandatory MATCH pattern.
    def test05_optional_expand_into(self):
        global redis_graph
        query = """MATCH (a)-[]->(b) OPTIONAL MATCH (a)-[e]->(b) RETURN a.v, b.v, TYPE(e) ORDER BY a.v, b.v"""
        actual_result = redis_graph.query(query)
        expected_result = [['v1', 'v2', 'E1'],
                           ['v2', 'v3', 'E2']]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # The OPTIONAL MATCH exactly repeats the MATCH, producing identical results.
        query_without_optional = """MATCH (a)-[e]->(b) RETURN a.v, b.v, TYPE(e) ORDER BY a.v, b.v"""
        result_without_optional = redis_graph.query(query_without_optional)
        self.env.assertEquals(actual_result.result_set, result_without_optional.result_set)

    # Optional MATCH clause with endpoints resolved by the mandatory MATCH pattern and new filters introduced.
    def test06_optional_expand_into_with_reltype(self):
        global redis_graph
        query = """MATCH (a)-[]->(b) OPTIONAL MATCH (a)-[e:E2]->(b) RETURN a.v, b.v, TYPE(e) ORDER BY a.v, b.v"""
        actual_result = redis_graph.query(query)
        # Only (v2)-[E2]->(v3) fulfills the constraint of the OPTIONAL MATCH clause.
        expected_result = [['v1', 'v2', None],
                           ['v2', 'v3', 'E2']]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Optional MATCH clause with endpoints resolved by the mandatory MATCH pattern, but no mandatory traversal.
    def test07_optional_expand_into_cartesian_product(self):
        global redis_graph
        query = """MATCH (a {v: 'v1'}), (b) OPTIONAL MATCH (a)-[e]->(b) RETURN a.v, b.v, TYPE(e) ORDER BY a.v, b.v"""
        actual_result = redis_graph.query(query)
        # All nodes are represented, but (v1)-[E1]->(v2) is the only matching connection.
        expected_result = [['v1', 'v1', None],
                           ['v1', 'v2', 'E1'],
                           ['v1', 'v3', None],
                           ['v1', 'v4', None]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # TODO ExpandInto doesn't evaluate bidirectionally properly
    # Optional MATCH clause with endpoints resolved by the mandatory MATCH pattern and a bidirectional optional pattern.
    #  def test08_optional_expand_into_bidirectional(self):
        #  global redis_graph
        #  query = """MATCH (a), (b {v: 'v2'}) OPTIONAL MATCH (a)-[e]-(b) RETURN a.v, b.v, TYPE(e) ORDER BY a.v, b.v"""
        #  actual_result = redis_graph.query(query)
        #  # All nodes are represented, but only edges with (v2) as an endpoint match.
        #  expected_result = [['v1', 'v2', 'E1'],
                           #  ['v2', 'v2', None],
                           #  ['v3', 'v2', 'E2'],
                           #  ['v3', 'v2', None]]
        #  self.env.assertEquals(actual_result.result_set, expected_result)

    # Optional MATCH clause with variable-length traversal and some results match.
    def test09_optional_variable_length(self):
        global redis_graph
        query = """MATCH (a) OPTIONAL MATCH (a)-[*]->(b) RETURN a.v, b.v ORDER BY a.v, b.v"""
        actual_result = redis_graph.query(query)
        expected_result = [['v1', 'v2'],
                           ['v1', 'v3'],
                           ['v2', 'v3'],
                           ['v3', None],
                           ['v4', None]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Optional MATCH clause with variable-length traversal and all results match.
    def test10_optional_variable_length_all_matches(self):
        global redis_graph
        query = """MATCH (a {v: 'v1'}) OPTIONAL MATCH (a)-[*]->(b) RETURN a.v, b.v ORDER BY a.v, b.v"""
        actual_result = redis_graph.query(query)
        expected_result = [['v1', 'v2'],
                           ['v1', 'v3']]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Optional MATCH clause with a variable-length traversal that has no matches.
    def test11_optional_variable_length_no_matches(self):
        global redis_graph
        query = """MATCH (a {v: 'v3'}) OPTIONAL MATCH (a)-[*]->(b) RETURN a.v, b.v ORDER BY a.v, b.v"""
        actual_result = redis_graph.query(query)
        expected_result = [['v3', None]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Multiple interdependent optional MATCH clauses.
    def test12_multiple_optional_traversals(self):
        global redis_graph
        query = """MATCH (a) OPTIONAL MATCH (a)-[]->(b) OPTIONAL MATCH (b)-[]->(c) RETURN a.v, b.v, c.v ORDER BY a.v, b.v, c.v"""
        actual_result = redis_graph.query(query)
        expected_result = [['v1', 'v2', 'v3'],
                           ['v2', 'v3', None],
                           ['v3', None, None],
                           ['v4', None, None]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Multiple interdependent optional MATCH clauses with both directed and bidirectional traversals.
    def test13_multiple_optional_multi_directional_traversals(self):
        global redis_graph
        query = """MATCH (a) OPTIONAL MATCH (a)-[]-(b) OPTIONAL MATCH (b)-[]->(c) RETURN a.v, b.v, c.v ORDER BY a.v, b.v, c.v"""
        actual_result = redis_graph.query(query)
        expected_result = [['v1', 'v2', 'v3'],
                           ['v2', 'v1', 'v2'],
                           ['v2', 'v3', None],
                           ['v3', 'v2', 'v3'],
                           ['v4', None, None]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Multiple interdependent optional MATCH clauses with exclusively bidirectional traversals.
    def test14_multiple_optional_bidirectional_traversals(self):
        global redis_graph
        query = """MATCH (a) OPTIONAL MATCH (a)-[]-(b) OPTIONAL MATCH (b)-[]-(c) RETURN a.v, b.v, c.v ORDER BY a.v, b.v, c.v"""
        actual_result = redis_graph.query(query)
        expected_result = [['v1', 'v2', 'v1'],
                           ['v1', 'v2', 'v3'],
                           ['v2', 'v1', 'v2'],
                           ['v2', 'v3', 'v2'],
                           ['v3', 'v2', 'v1'],
                           ['v3', 'v2', 'v3'],
                           ['v4', None, None]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Build a named path in an optional clause.
    def test15_optional_named_path(self):
        global redis_graph
        query = """MATCH (a) OPTIONAL MATCH p = (a)-[]->(b) RETURN length(p) ORDER BY length(p)"""
        actual_result = redis_graph.query(query)
        # 2 nodes have outgoing edges and 2 do not, so expected 2 paths of length 1 and 2 null results.
        expected_result = [[1],
                           [1],
                           [None],
                           [None]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Return a result set with null values in the first record and non-null values in subsequent records.
    def test16_optional_null_first_result(self):
        global redis_graph
        query = """MATCH (a) OPTIONAL MATCH (a)-[e]->(b) RETURN a, b, TYPE(e) ORDER BY EXISTS(b), a.v, b.v"""
        actual_result = redis_graph.query(query)
        expected_result = [[nodes['v3'], None, None],
                           [nodes['v4'], None, None],
                           [nodes['v1'], nodes['v2'], 'E1'],
                           [nodes['v2'], nodes['v3'], 'E2']]
        self.env.assertEquals(actual_result.result_set, expected_result)
