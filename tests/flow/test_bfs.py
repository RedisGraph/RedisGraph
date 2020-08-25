import os
import sys
from RLTest import Env
from redisgraph import Graph, Node, Edge
from base import FlowTestsBase

graph = None
nodes = {}

class testBFS(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        global graph
        redis_con = self.env.getConnection()
        graph = Graph("proc_bfs", redis_con)
        self.populate_graph()

    def populate_graph(self):
        global nodes
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

        edge = Edge(nodes['a'], "E1", nodes['b'])
        graph.add_edge(edge)

        edge = Edge(nodes['b'], "E1", nodes['c'])
        graph.add_edge(edge)

        edge = Edge(nodes['b'], "E2", nodes['d'])
        graph.add_edge(edge)

        edge = Edge(nodes['d'], "E1", nodes['e'])
        graph.add_edge(edge)

        graph.flush()

    # Test BFS from a single source without specifying a relationship type.
    def test01_bfs_single_source_all_reltypes(self):
        global graph
        # Test BFS reachability algorithm
        query = """MATCH (a {v: 'a'}) CALL algo.BFS(a, 0, NULL) YIELD node, level RETURN node.v, level ORDER BY node.v"""
        actual_result = graph.query(query)
        expected_result = [['a', 1],
                           ['b', 2],
                           ['c', 3],
                           ['d', 3],
                           ['e', 4]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test BFS path-tracking algorithm
        query = """MATCH (a {v: 'a'}) CALL algo.BFSTree(a, 0, NULL) YIELD node, level, path RETURN node.v, level, path ORDER BY node.v"""
        actual_result = graph.query(query)
        expected_result = [['a', 1, [nodes['a']]],
                           ['b', 2, [nodes['a'], nodes['b']]],
                           ['c', 3, [nodes['a'], nodes['b'], nodes['c']]],
                           ['d', 3, [nodes['a'], nodes['b'], nodes['d']]],
                           ['e', 4, [nodes['a'], nodes['b'], nodes['d'], nodes['e']]]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Test BFS from a single source traversing a single relationship type.
    def test02_bfs_single_source_restricted_reltype(self):
        global graph
        # Test BFS reachability algorithm
        query = """MATCH (a {v: 'a'}) CALL algo.BFS(a, 0, 'E1') YIELD node, level RETURN node.v, level ORDER BY node.v"""
        actual_result = graph.query(query)
        expected_result = [['a', 1],
                           ['b', 2],
                           ['c', 3]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test BFS path-tracking algorithm
        query = """MATCH (a {v: 'a'}) CALL algo.BFSTree(a, 0, 'E1') YIELD node, level, path RETURN node.v, level, path ORDER BY node.v"""
        actual_result = graph.query(query)
        expected_result = [['a', 1, [nodes['a']]],
                           ['b', 2, [nodes['a'], nodes['b']]],
                           ['c', 3, [nodes['a'], nodes['b'], nodes['c']]]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Test BFS from all sources traversing a single relationship type, ignoring 0-hop paths.
    def test03_bfs_all_sources_restricted_reltype(self):
        global graph
        # We do not expect to see 'd' as a destination or intermediate node, as it's connected by an 'E2' edge.
        query = """MATCH (a) CALL algo.BFS(a, 0, 'E1') YIELD node, level WHERE level > 1 RETURN node.v, level ORDER BY node.v, level"""
        actual_result = graph.query(query)
        expected_result = [['b', 2],
                           ['c', 2],
                           ['c', 3],
                           ['e', 2]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test BFS path-tracking algorithm
        query = """MATCH (a) CALL algo.BFSTree(a, 0, 'E1') YIELD node, level, path WHERE level > 1 RETURN node.v, level, path ORDER BY node.v, level"""
        actual_result = graph.query(query)
        expected_result = [['b', 2, [nodes['a'], nodes['b']]],
                           ['c', 2, [nodes['b'], nodes['c']]],
                           ['c', 3, [nodes['a'], nodes['b'], nodes['c']]],
                           ['e', 2, [nodes['d'], nodes['e']]]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Test BFS from a single source with a maximum depeth.
    def test04_bfs_single_source_max_depth(self):
        # Test BFS reachability algorithm
        query = """MATCH (a {v: 'a'}) CALL algo.BFS(a, 2, NULL) YIELD node, level RETURN node.v, level ORDER BY node.v"""
        actual_result = graph.query(query)
        expected_result = [['a', 1],
                           ['b', 2]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test BFS path-tracking algorithm
        query = """MATCH (a {v: 'a'}) CALL algo.BFSTree(a, 2, NULL) YIELD node, level, path RETURN node.v, level, path ORDER BY node.v"""
        actual_result = graph.query(query)
        expected_result = [['a', 1, [nodes['a']]],
                           ['b', 2, [nodes['a'], nodes['b']]]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Test BFS from all sources with a maximum depeth.
    def test05_bfs_all_sources_max_depth(self):
        # Test BFS reachability algorithm
        query = """MATCH (a) CALL algo.BFS(a, 2, NULL) YIELD node, level RETURN node.v, level ORDER BY node.v, level"""
        actual_result = graph.query(query)
        expected_result = [['a', 1],
                           ['b', 1],
                           ['b', 2],
                           ['c', 1],
                           ['c', 2],
                           ['d', 1],
                           ['d', 2],
                           ['e', 1],
                           ['e', 2]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test BFS path-tracking algorithm
        query = """MATCH (a) CALL algo.BFSTree(a, 2, NULL) YIELD node, level, path RETURN node.v, level, path ORDER BY node.v, level"""
        actual_result = graph.query(query)
        expected_result = [['a', 1, [nodes['a']]],
                           ['b', 1, [nodes['b']]],
                           ['b', 2, [nodes['a'], nodes['b']]],
                           ['c', 1, [nodes['c']]],
                           ['c', 2, [nodes['b'], nodes['c']]],
                           ['d', 1, [nodes['d']]],
                           ['d', 2, [nodes['b'], nodes['d']]],
                           ['e', 1, [nodes['e']]],
                           ['e', 2, [nodes['d'], nodes['e']]]]
        self.env.assertEquals(actual_result.result_set, expected_result)
