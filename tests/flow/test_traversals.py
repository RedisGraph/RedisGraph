import os
import sys
import unittest
from redisgraph import Graph, Node, Edge

# import redis
from .disposableredis import DisposableRedis

from base import FlowTestsBase

redis_graph = None

nodevals = ['a', 'b', 'c']
edge_connections = [[0, 1], [1, 2], [1, 0]]

def redis():
    return DisposableRedis(loadmodule=os.path.dirname(os.path.abspath(__file__)) + '/../../src/redisgraph.so')

class GraphTraversalsFlowTest(FlowTestsBase):
    @classmethod
    def setUpClass(cls):
        print "GraphTraversalsFlowTest"
        global redis_graph
        cls.r = redis()
        cls.r.start()
        redis_con = cls.r.client()
        redis_graph = Graph("G", redis_con)

        cls.populate_graph()

    @classmethod
    def tearDownClass(cls):
        cls.r.stop()
        # pass

    @classmethod
    def populate_graph(cls):
        global redis_graph

        # Create graph with 3 edges and one 2-hop cycle
        # (Values are the properties on each entity)
        # (a)-[0]->(b)-[1]->(c)
        # (b)-[2]->(a)
        nodes = {}
         # Create entities
        for idx, n in enumerate(nodevals):
            node = Node(label="nodeval", properties={"val": n})
            redis_graph.add_node(node)
            nodes[idx] = node

        for idx, e in enumerate(edge_connections):
            edge = Edge(nodes[e[0]], "connects", nodes[e[1]], {"edgeprop": idx})
            redis_graph.add_edge(edge)

        redis_graph.commit()

    # Verify that node properties match expectations
    def test_node_properties(self):
        query = """MATCH(a) return a ORDER BY a.val"""
        actual_result = redis_graph.query(query)
        assert (len(actual_result.result_set)-1 == (len(nodevals)))
        for idx, result in enumerate(actual_result.result_set[1:]):
            assert result[0] == nodevals[idx]

    # Verify that edge properties match expectations
    def test_edge_properties(self):
        query = """MATCH()-[b]->() return b ORDER BY b.edgeprop"""
        actual_result = redis_graph.query(query)
        assert (len(actual_result.result_set)-1 == (len(edge_connections)))
        for idx, result in enumerate(actual_result.result_set[1:]):
            assert (int(float(result[0])) == idx)

    # Verify that the appropriate nodes are connected
    def test_single_hop(self):
        query = """MATCH(a)-[]->(b) return a, b ORDER BY a.val, b.val"""
        actual_result = redis_graph.query(query)
        assert (len(actual_result.result_set)-1 == (len(edge_connections)))
        assert (actual_result.result_set[1:]) == [['a', 'b'], ['b', 'a'], ['b', 'c']]

    # Verify that the appropriate nodes are connected with all labels specified
    def test_single_labeled_hop(self):
        query = """MATCH(a:nodeval)-[:connects]->(b:nodeval) return a, b ORDER BY a.val, b.val"""
        actual_result = redis_graph.query(query)
        assert (len(actual_result.result_set)-1 == (len(edge_connections)))
        assert (actual_result.result_set[1:]) == [['a', 'b'], ['b', 'a'], ['b', 'c']]

    # Verify that no node is connected to itself with all labels specified
    def test_single_labeled_hop_loop_detection(self):
        query = """MATCH(a:nodeval)-[b:connects]->(a:nodeval) return a,b"""
        actual_result = redis_graph.query(query)
        assert (len(actual_result.result_set)-1 == 0)

    # Verify that no node is directly connected to itself
    def test_single_hop_loop_detection(self):
        query = """MATCH(a)-[]->(a) return a"""
        actual_result = redis_graph.query(query)
        print actual_result.result_set
        assert (len(actual_result.result_set)-1 == 0)

    # Verify that two nodes are connected to themselves through a variable-length traversal
    def test_single_labeled_hop_loop_detection(self):
        query = """MATCH(a)-[*]->(a) return a ORDER BY a.val"""
        actual_result = redis_graph.query(query)
        print actual_result.result_set
        assert (actual_result.result_set[1:]) == [['a'], ['b']]

if __name__ == '__main__':
    unittest.main()
