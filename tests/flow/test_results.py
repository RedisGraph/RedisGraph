import os
import sys
import unittest
from redisgraph import Graph, Node, Edge

# import redis
from .disposableredis import DisposableRedis

from base import FlowTestsBase

graph = None
redis_con = None

people = ["Roi", "Alon", "Ailon", "Boaz"]

def redis():
    return DisposableRedis(loadmodule=os.path.dirname(os.path.abspath(__file__)) + '/../../src/redisgraph.so')

class ResultSetFlowTest(FlowTestsBase):
    @classmethod
    def setUpClass(cls):
        print "ResultSetFlowTest"
        global graph
        global redis_con
        cls.r = redis()
        cls.r.start()
        redis_con = cls.r.client()
        graph = Graph("G", redis_con)
        cls.populate_graph()

    @classmethod
    def tearDownClass(cls):
        cls.r.stop()
        # pass

    @classmethod
    def populate_graph(cls):
        global graph
        nodes = {}
        # Create entities
        for idx, p in enumerate(people):
            node = Node(label="person", properties={"name": p, "val": idx})
            graph.add_node(node)
            nodes[p] = node

        # Fully connected graph
        for src in nodes:
            for dest in nodes:
                if src != dest:
                    edge = Edge(nodes[src], "know", nodes[dest])
                    graph.add_edge(edge)

        graph.commit()


    # Verify that scalar returns function properly
    def test01_return_scalars(self):
        query = """MATCH (a) RETURN a.name, a.val ORDER BY a.val"""
        result = graph.query(query)

        expected_result = [['a.name', 'a.val'],
                           ['Roi', '0'],
                           ['Alon', '1'],
                           ['Ailon', '2'],
                           ['Boaz', '3']]

        assert(len(result.result_set) == 5) # Header row + 4 nodes
        assert(len(result.result_set[0]) == 2) # 2 columns in result set
        assert(result.result_set == expected_result)

    # Verify that full node returns function properly
    def test02_return_nodes(self):
        query = """MATCH (a) RETURN a"""
        result = graph.query(query)

        # TODO add more assertions after updated client format is defined
        assert(len(result.result_set) == 5) # Header row + 4 nodes
        assert(len(result.result_set[0]) == 1) # 1 column in result set

    # Verify that full edge returns function properly
    def test03_return_edges(self):
        query = """MATCH ()-[e]->() RETURN e"""
        result = graph.query(query)

        # TODO add more assertions after updated client format is defined
        assert(len(result.result_set) == 13) # Header row + 12 relations (fully connected graph)
        assert(len(result.result_set[0]) == 1) # 1 column in result set

    def test04_mixed_returns(self):
        query = """MATCH (a)-[e]->() RETURN a.name, a, e ORDER BY a.val"""
        result = graph.query(query)

        # TODO add more assertions after updated client format is defined
        assert(len(result.result_set) == 13) # Header row + 12 relations (fully connected graph)
        assert(len(result.result_set[0]) == 3) # 3 columns in result set


    # Verify that the DISTINCT operator works with full entity returns
    def test05_distinct_full_entities(self):
        graph2 = Graph("H", redis_con)
        query = """CREATE (a)-[:e]->(), (a)-[:e]->()"""
        result = graph2.query(query)
        assert(result.nodes_created == 3)
        assert(result.relationships_created == 2)

        query = """MATCH (a)-[]->() RETURN a"""
        non_distinct = graph2.query(query)
        query = """MATCH (a)-[]->() RETURN DISTINCT a"""
        distinct = graph2.query(query)

        assert(len(non_distinct.result_set) == 3)
        assert(len(distinct.result_set) == 2)
