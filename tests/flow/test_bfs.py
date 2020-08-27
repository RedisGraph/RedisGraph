import os
import sys
from RLTest import Env
from redisgraph import Graph, Node, Edge
from base import FlowTestsBase

graph = None
nodes = {}
edges = {}

class testBFS(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        global graph
        redis_con = self.env.getConnection()
        graph = Graph("proc_bfs", redis_con)
        self.populate_graph()

    def populate_graph(self):
        global nodes
        global edges 
        # Construct a graph with the form:
        # (a)-[:E1]->(b:B)-[:E1]->(c), (b)-[:E2]->(d)-[:E1]->(e)
        nodes['a'] = Node(label="A", properties={"v": 'a'})
        graph.add_node(nodes['a'])

        nodes['b'] = Node(label="A", properties={"v": 'b'})
        graph.add_node(nodes['b'])

        nodes['c'] = Node(label="A", properties={"v": 'c'})
        graph.add_node(nodes['c'])

        nodes['d'] = Node(label="A", properties={"v": 'd'})
        graph.add_node(nodes['d'])

        nodes['e'] = Node(label="A", properties={"v": 'e'})
        graph.add_node(nodes['e'])

        # Edges have the same property as their destination
        edges[0] = Edge(nodes['a'], "E1", nodes['b'], properties={"v": 'b'})
        graph.add_edge(edges[0])

        edges[1] = Edge(nodes['b'], "E1", nodes['c'], properties={"v": 'c'})
        graph.add_edge(edges[1])

        edges[2] = Edge(nodes['b'], "E2", nodes['d'], properties={"v": 'd'})
        graph.add_edge(edges[2])

        edges[3] = Edge(nodes['d'], "E1", nodes['e'], properties={"v": 'e'})
        graph.add_edge(edges[3])

        graph.flush()

    # Verify that 
    def compare_unsorted_arrays(self, a, b):
        self.env.assertEquals(len(a), len(b))
        for elem in a:
            # Each element in a should appear in b exactly once.
            self.env.assertEquals(b.count(elem), 1)


    # Test BFS from a single source without specifying a relationship type.
    def test01_bfs_single_source_all_reltypes(self):
        global graph
        # Test BFS algorithm for node collection.
        # The results array must be sorted, since the order is non-deterministic (due to creations occurring in any order).
        query = """MATCH (a {v: 'a'}) CALL algo.BFS(a, 0, NULL, false) YIELD nodes UNWIND nodes AS n WITH n.v AS v ORDER BY n.v RETURN COLLECT(v)"""
        actual_result = graph.query(query)
        expected_result = [[['b', 'c', 'd', 'e']]]
        self.env.assertEquals(actual_result.result_set, expected_result)
        query = """MATCH (a {v: 'a'}) CALL algo.BFS(a, 0, NULL, false) YIELD nodes RETURN nodes"""
        self.compare_unsorted_arrays(actual_result.result_set[0][0], expected_result[0][0])

        query = """MATCH (a {v: 'a'}) CALL algo.BFS(a, 0, NULL, false) YIELD start_node, nodes UNWIND nodes AS n WITH start_node, n.v AS v ORDER BY n.v RETURN start_node.v, COLLECT(v)"""
        actual_result = graph.query(query)
        expected_result = [['a', ['b', 'c', 'd', 'e']]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test BFS algorithm for node and edge collection.
        # Parity between nodes and edges can be validated by testing the properties of each for equality,
        # as edges have the same property as their destination node.
        query = """MATCH (a {v: 'a'}) CALL algo.BFS(a, 0, NULL, true) YIELD nodes, edges RETURN [n IN nodes | n.v], [e IN edges | e.v]"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], actual_result.result_set[0][1])
        #  import ipdb
        #  ipdb.set_trace()

    # Test BFS from a single source traversing a single relationship type.
    def test02_bfs_single_source_restricted_reltype(self):
        global graph
        # Test BFS algorithm for node collection.
        query = """MATCH (a {v: 'a'}) CALL algo.BFS(a, 0, 'E1', false) YIELD nodes RETURN [n IN nodes | n.v]"""
        actual_result = graph.query(query)
        expected_result = ['b', 'c']
        self.compare_unsorted_arrays(actual_result.result_set[0][0], expected_result)

        # Test BFS algorithm for node and edge collection.
        query = """MATCH (a {v: 'a'}) CALL algo.BFS(a, 0, 'E1', true) YIELD nodes, edges RETURN [n IN nodes | n.v], [e IN edges | e.v]"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], actual_result.result_set[0][1])

    #  # Test BFS from all sources traversing a single relationship type, ignoring 0-hop paths.
    def test03_bfs_all_sources_restricted_reltype(self):
        global graph
        # We only expect to see 'd' as a source node, as it is connected as a destination by an 'E2' edge.
        query = """MATCH (a) CALL algo.BFS(a, 0, 'E1', false) YIELD start_node, nodes RETURN start_node.v, [n IN nodes | n.v] ORDER BY start_node.v"""
        actual_result = graph.query(query)
        expected_result = [['a', ['b', 'c']],
                           ['b', ['c']],
                           ['d', ['e']]]
        for idx, row in enumerate(actual_result.result_set):
            self.env.assertEquals(row[0], expected_result[idx][0])
            self.compare_unsorted_arrays(row[1], expected_result[idx][1])

        # Test BFS path-tracking algorithm
        query = """MATCH (a) CALL algo.BFS(a, 0, 'E1', true) YIELD start_node, nodes, edges RETURN start_node.v, [n IN nodes | n.v], [e IN edges | e.v] ORDER BY start_node.v"""

        actual_result = graph.query(query)
        expected_result = [['a', ['b', 'c'], ['b', 'c']],
                           ['b', ['c'], ['c']],
                           ['d', ['e'], ['e']]]
        for idx, row in enumerate(actual_result.result_set):
            self.env.assertEquals(row[0], expected_result[idx][0])
            self.compare_unsorted_arrays(row[1], expected_result[idx][1])
            self.compare_unsorted_arrays(row[2], expected_result[idx][2])

    #  # Test BFS from a single source with a maximum depth.
    def test04_bfs_single_source_max_depth(self):
        query = """MATCH (a {v: 'a'}) CALL algo.BFS(a, 2, NULL, false) YIELD nodes RETURN [n IN nodes | n.v]"""
        actual_result = graph.query(query)
        expected_result = [[['b']]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """MATCH (a {v: 'a'}) CALL algo.BFS(a, 2, NULL, true) YIELD nodes, edges RETURN [n IN nodes | n.v], [e IN edges | e.v]"""
        actual_result = graph.query(query)
        expected_result = [[['b'], ['b']]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    #  # Test BFS from all sources with a maximum depeth.
    def test05_bfs_all_sources_max_depth(self):
        query = """MATCH (a) CALL algo.BFS(a, 2, NULL, false) YIELD start_node, nodes RETURN start_node.v, [n IN nodes | n.v] ORDER BY start_node.v"""
        actual_result = graph.query(query)
        expected_result = [['a', ['b']],
                           ['b', ['c', 'd']],
                           ['d', ['e']]]
        for idx, row in enumerate(actual_result.result_set):
            self.env.assertEquals(row[0], expected_result[idx][0])
            self.compare_unsorted_arrays(row[1], expected_result[idx][1])

        query = """MATCH (a) CALL algo.BFS(a, 2, NULL, true) YIELD start_node, nodes, edges RETURN start_node.v, [n IN nodes | n.v], [e IN edges | e.v] ORDER BY start_node.v"""
        actual_result = graph.query(query)
        expected_result = [['a', ['b'], ['b']],
                           ['b', ['c', 'd'], ['c', 'd']],
                           ['d', ['e'], ['e']]]
        for idx, row in enumerate(actual_result.result_set):
            self.env.assertEquals(row[0], expected_result[idx][0])
            self.compare_unsorted_arrays(row[1], expected_result[idx][1])
            self.compare_unsorted_arrays(row[2], expected_result[idx][2])
