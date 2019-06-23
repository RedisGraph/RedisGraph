from base import FlowTestsBase
from disposableredis import DisposableRedis
import os
import sys
import unittest
from redisgraph import Graph, Node, Edge
import redis

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
graph = None
redis_con = None
people = ["Roi", "Alon", "Ailon", "Boaz"]

def disposable_redis():
    return DisposableRedis(loadmodule=os.path.dirname(os.path.abspath(__file__)) + '/../../src/redisgraph.so')

class OptimizationsPlanTest(FlowTestsBase):
    @classmethod
    def setUpClass(cls):
        print "OptimizationsPlanTest"
        global graph
        cls.r = disposable_redis()
        cls.r.start()
        redis_con = cls.r.client()
        graph = Graph("g", redis_con)
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

    def test_typeless_edge_count(self):
        query = """MATCH ()-[r]->() RETURN COUNT(r)"""
        resultset = graph.query(query).result_set
        executionPlan = graph.execution_plan(query)
        self.assertIn("Project", executionPlan)
        self.assertIn("Results", executionPlan)
        self.assertNotIn("All Node Scan", executionPlan)
        self.assertNotIn("Conditional Traverse", executionPlan)
        self.assertNotIn("Aggregate", executionPlan)
        expected = [[12]]
        self.assertEqual(resultset, expected)
    
    def test_typeless_edge_count_with_alias(self):
        query = """MATCH ()-[r]->() RETURN COUNT(r) as c"""
        resultset = graph.query(query).result_set
        executionPlan = graph.execution_plan(query)
        self.assertIn("Project", executionPlan)
        self.assertIn("Results", executionPlan)
        self.assertNotIn("All Node Scan", executionPlan)
        self.assertNotIn("Conditional Traverse", executionPlan)
        self.assertNotIn("Aggregate", executionPlan)
        expected = [[12]]
        self.assertEqual(resultset, expected)

    def test_non_labeled_node_count(self):
        query = """MATCH (n) RETURN COUNT(n)"""
        resultset = graph.query(query).result_set
        executionPlan = graph.execution_plan(query)
        self.assertIn("Project", executionPlan)
        self.assertIn("Results", executionPlan)
        self.assertNotIn("All Node Scan", executionPlan)
        self.assertNotIn("Node By Label Scan", executionPlan)
        self.assertNotIn("Aggregate", executionPlan)
        expected = [[4]]
        self.assertEqual(resultset, expected)
    
    def test_non_labeled_node_count_with_alias(self):
        query = """MATCH (n) RETURN COUNT(n) as c"""
        resultset = graph.query(query).result_set
        executionPlan = graph.execution_plan(query)
        self.assertIn("Project", executionPlan)
        self.assertIn("Results", executionPlan)
        self.assertNotIn("All Node Scan", executionPlan)
        self.assertNotIn("Node By Label Scan", executionPlan)
        self.assertNotIn("Aggregate", executionPlan)
        expected = [[4]]
        self.assertEqual(resultset, expected)
    
    def test_labled_node_count(self):
        query = """MATCH (n:person) RETURN COUNT(n)"""
        resultset = graph.query(query).result_set
        executionPlan = graph.execution_plan(query)
        self.assertIn("Project", executionPlan)
        self.assertIn("Results", executionPlan)
        self.assertNotIn("All Node Scan", executionPlan)
        self.assertNotIn("Node By Label Scan", executionPlan)
        self.assertNotIn("Aggregate", executionPlan)
        expected = [[4]]
        self.assertEqual(resultset, expected)



