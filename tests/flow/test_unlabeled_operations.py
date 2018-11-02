import os
import sys
import unittest
from redisgraph import Graph, Node, Edge

# import redis
from .disposableredis import DisposableRedis

from base import FlowTestsBase

redis_graph = None
male = ["Roi", "Alon", "Omri"]
female = ["Hila", "Lucy"]

def redis():
    return DisposableRedis(loadmodule=os.path.dirname(os.path.abspath(__file__)) + '/../../src/redisgraph.so')

class UnlabeledOperationsFlowTest(FlowTestsBase):
    @classmethod
    def setUpClass(cls):
        print "UnlabeledOperationsFlowTest"
        global redis_graph
        cls.r = redis()
        cls.r.start()
        redis_con = cls.r.client()
        redis_graph = Graph("G", redis_con)

        cls.populate_graph()

    @classmethod
    def tearDownClass(cls):
        cls.r.stop()

    @classmethod
    def populate_graph(cls):
        redis_graph
        
        nodes = {}
         # Create entities
        
        for m in male:
            node = Node(label="male", properties={"name": m, "gender": "M"})
            redis_graph.add_node(node)
            nodes[m] = node
        
        for f in female:
            node = Node(label="female", properties={"name": f, "gender": "F"})
            redis_graph.add_node(node)
            nodes[f] = node

        for n in nodes:
            for m in nodes:
                if n == m: continue
                edge = Edge(nodes[n], "knows", nodes[m])
                redis_graph.add_edge(edge)

        redis_graph.commit()

    def test01_unlabeled_property_additions(self):
        query = """MATCH (a) SET a.newprop = a.gender"""
        actual_result = redis_graph.query(query)
        assert (actual_result.properties_set == 5)

    def test02_unlabeled_schema_after_addition(self):
        query = """MATCH (a) RETURN a"""
        actual_result = redis_graph.query(query)
        assert "a.newprop" in actual_result.result_set[0]
        for entry in actual_result.result_set:
            assert (len(entry) == 3)

    def test03_labeled_schema_after_addition(self):
        query = """MATCH (a:female) RETURN a"""
        actual_result = redis_graph.query(query)
        assert "a.newprop" in actual_result.result_set[0]
        for entry in actual_result.result_set:
            assert (len(entry) == 3)


if __name__ == '__main__':
    unittest.main()