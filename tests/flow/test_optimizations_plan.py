from base import FlowTestsBase
import os
import sys
from redisgraph import Graph, Node, Edge


sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
graph = None
redis_con = None
people = ["Roi", "Alon", "Ailon", "Boaz"]


class testOptimizationsPlan(FlowTestsBase):
    def __init__(self):
        super(testOptimizationsPlan, self).__init__()
        global graph
        global redis_con
        redis_con = self.env.getConnection()
        graph = Graph("g", redis_con)
        self.populate_graph()

    def populate_graph(self):
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
        
        for src in nodes:
            for dest in nodes:
                if src != dest:
                    edge = Edge(nodes[src], "works_with", nodes[dest])
                    graph.add_edge(edge)

        graph.commit()
        query = """MATCH (a)-[:know]->(b) CREATE (a)-[:know]->(b)"""
        graph.query(query)

    def test_typeless_edge_count(self):
        query = """MATCH ()-[r]->() RETURN COUNT(r)"""
        resultset = graph.query(query).result_set
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Project", executionPlan)
        self.env.assertIn("Results", executionPlan)
        self.env.assertNotIn("All Node Scan", executionPlan)
        self.env.assertNotIn("Conditional Traverse", executionPlan)
        self.env.assertNotIn("Aggregate", executionPlan)
        expected = [[36]]
        self.env.assertEqual(resultset, expected)
    
    def test_typed_edge_count(self):
        query = """MATCH ()-[r:know]->() RETURN COUNT(r)"""
        resultset = graph.query(query).result_set
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Project", executionPlan)
        self.env.assertIn("Results", executionPlan)
        self.env.assertNotIn("All Node Scan", executionPlan)
        self.env.assertNotIn("Conditional Traverse", executionPlan)
        self.env.assertNotIn("Aggregate", executionPlan)
        expected = [[24]]
        self.env.assertEqual(resultset, expected)
    
    def test_typeless_edge_count_with_alias(self):
        query = """MATCH ()-[r]->() RETURN COUNT(r) as c"""
        resultset = graph.query(query).result_set
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Project", executionPlan)
        self.env.assertIn("Results", executionPlan)
        self.env.assertNotIn("All Node Scan", executionPlan)
        self.env.assertNotIn("Conditional Traverse", executionPlan)
        self.env.assertNotIn("Aggregate", executionPlan)
        expected = [[36]]
        self.env.assertEqual(resultset, expected)
    
    def test_typed_edge_count_with_alias(self):
        query = """MATCH ()-[r:know]->() RETURN COUNT(r) as c"""
        resultset = graph.query(query).result_set
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Project", executionPlan)
        self.env.assertIn("Results", executionPlan)
        self.env.assertNotIn("All Node Scan", executionPlan)
        self.env.assertNotIn("Conditional Traverse", executionPlan)
        self.env.assertNotIn("Aggregate", executionPlan)
        expected = [[24]]
        self.env.assertEqual(resultset, expected)

    def test_multiple_typed_edge_count_with_alias(self):
        query = """MATCH ()-[r:know | :works_with]->() RETURN COUNT(r) as c"""
        resultset = graph.query(query).result_set
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Project", executionPlan)
        self.env.assertIn("Results", executionPlan)
        self.env.assertNotIn("All Node Scan", executionPlan)
        self.env.assertNotIn("Conditional Traverse", executionPlan)
        self.env.assertNotIn("Aggregate", executionPlan)
        expected = [[36]]
        self.env.assertEqual(resultset, expected)

    def test_non_labeled_node_count(self):
        query = """MATCH (n) RETURN COUNT(n)"""
        resultset = graph.query(query).result_set
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Project", executionPlan)
        self.env.assertIn("Results", executionPlan)
        self.env.assertNotIn("All Node Scan", executionPlan)
        self.env.assertNotIn("Node By Label Scan", executionPlan)
        self.env.assertNotIn("Aggregate", executionPlan)
        expected = [[4]]
        self.env.assertEqual(resultset, expected)
    
    def test_non_labeled_node_count_with_alias(self):
        query = """MATCH (n) RETURN COUNT(n) as c"""
        resultset = graph.query(query).result_set
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Project", executionPlan)
        self.env.assertIn("Results", executionPlan)
        self.env.assertNotIn("All Node Scan", executionPlan)
        self.env.assertNotIn("Node By Label Scan", executionPlan)
        self.env.assertNotIn("Aggregate", executionPlan)
        expected = [[4]]
        self.env.assertEqual(resultset, expected)
    
    def test_labled_node_count(self):
        query = """MATCH (n:person) RETURN COUNT(n)"""
        resultset = graph.query(query).result_set
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Project", executionPlan)
        self.env.assertIn("Results", executionPlan)
        self.env.assertNotIn("All Node Scan", executionPlan)
        self.env.assertNotIn("Node By Label Scan", executionPlan)
        self.env.assertNotIn("Aggregate", executionPlan)
        expected = [[4]]
        self.env.assertEqual(resultset, expected)
