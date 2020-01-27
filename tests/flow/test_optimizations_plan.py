from base import FlowTestsBase
import os
import sys
from redisgraph import Graph, Node, Edge


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

    def test01_typeless_edge_count(self):
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

    def test02_typed_edge_count(self):
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

    def test03_unknown_typed_edge_count(self):
        query = """MATCH ()-[r:unknown]->() RETURN COUNT(r)"""
        resultset = graph.query(query).result_set
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Project", executionPlan)
        self.env.assertIn("Results", executionPlan)
        self.env.assertNotIn("All Node Scan", executionPlan)
        self.env.assertNotIn("Conditional Traverse", executionPlan)
        self.env.assertNotIn("Aggregate", executionPlan)
        expected = [[0]]
        self.env.assertEqual(resultset, expected)

    def test04_typeless_edge_count_with_alias(self):
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

    def test05_typed_edge_count_with_alias(self):
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

    def test06_multiple_typed_edge_count_with_alias(self):
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

    def test07_count_unreferenced_edge(self):
        query = """MATCH ()-[:know]->(b) RETURN COUNT(b)"""
        # This count in this query cannot be reduced, as the traversal op doesn't store
        # data about non-referenced edges.
        resultset = graph.query(query).result_set
        executionPlan = graph.execution_plan(query)
        # Verify that the optimization was not applied.
        self.env.assertNotIn("Project", executionPlan)
        self.env.assertIn("Aggregate", executionPlan)
        self.env.assertIn("All Node Scan", executionPlan)
        self.env.assertIn("Conditional Traverse", executionPlan)
        expected = [[12]]
        self.env.assertEqual(resultset, expected)

    def test08_non_labeled_node_count(self):
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

    def test09_non_labeled_node_count_with_alias(self):
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

    def test10_labled_node_count(self):
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

    def test11_value_hash_join(self):
        # Issue a query that joins two streams on a node property.
        query = """MATCH (p1:person)-[:know]->({name: 'Roi'}), (p2)-[]->(:person {name: 'Alon'}) WHERE p1.name = p2.name RETURN p2.name ORDER BY p2.name"""
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Value Hash Join", executionPlan)
        self.env.assertNotIn("Cartesian Product", executionPlan)

        resultset = graph.query(query).result_set
        expected = [['Ailon'], ['Boaz']]
        self.env.assertEqual(resultset, expected)

        # Issue a query that joins two streams on a function call.
        query = """MATCH (p1:person)-[:know]->({name: 'Roi'}) MATCH (p2)-[]->(:person {name: 'Alon'}) WHERE ID(p1) = ID(p2) RETURN p2.name ORDER BY p2.name"""
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Value Hash Join", executionPlan)
        self.env.assertNotIn("Cartesian Product", executionPlan)

        resultset = graph.query(query).result_set
        self.env.assertEqual(resultset, expected) # same results expected

        # Validate identical results in a query that doesn't leverage this optimization.
        # TODO this query could in the future be optimized with a "Node Hash Join"
        query = """MATCH (p1:person)-[:know]->({name: 'Roi'}) MATCH (p2)-[]->(:person {name: 'Alon'}) WHERE p1 = p2 RETURN p2.name ORDER BY p2.name"""
        executionPlan = graph.execution_plan(query)
        self.env.assertNotIn("Value Hash Join", executionPlan)
        self.env.assertIn("Cartesian Product", executionPlan)

        resultset = graph.query(query).result_set
        self.env.assertEqual(resultset, expected) # same results expected

    def test12_multiple_stream_value_hash_join(self):
        # Issue a query that joins three streams.
        query = """MATCH (p1:person)-[:know]->({name: 'Roi'}), (p2)-[]->(:person {name: 'Alon'}), (p3) WHERE p1.name = p2.name AND ID(p2) = ID(p3) RETURN p2.name ORDER BY p2.name"""
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Value Hash Join", executionPlan)
        self.env.assertNotIn("Cartesian Product", executionPlan)

        resultset = graph.query(query).result_set
        expected = [['Ailon'], ['Boaz']]
        self.env.assertEqual(resultset, expected)

        # Issue a query that joins four streams that all resolve the same entity.
        query = """MATCH (p1 {name: 'Ailon'}), (p2), (p3), (p4) WHERE ID(p1) = ID(p2) AND ID(p2) = ID(p3) AND p3.name = p4.name RETURN p4.name"""
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Value Hash Join", executionPlan)
        self.env.assertNotIn("Cartesian Product", executionPlan)

        expected = [['Ailon']]
        resultset = graph.query(query).result_set
        self.env.assertEqual(resultset, expected)

        # Issue a query that joins four streams that all resolve the same entity, with multiple reapeating filter (issue #869).
        query = """MATCH (p1 {name: 'Ailon'}), (p2), (p3), (p4) WHERE ID(p1) = ID(p2) AND ID(p2) = ID(p3) AND ID(p3)=ID(p2) AND ID(p2)= ID(p1) AND p3.name = p4.name AND p4.name = p3.name RETURN p4.name"""
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Value Hash Join", executionPlan)
        self.env.assertNotIn("Cartesian Product", executionPlan)

        expected = [['Ailon']]
        resultset = graph.query(query).result_set
        self.env.assertEqual(resultset, expected)

    def test13_distinct_aggregations(self):
        # Verify that the Distinct operation is removed from the aggregating query.
        query = """MATCH (src:person)-[:know]->(dest) RETURN DISTINCT src.name, COUNT(dest) ORDER BY src.name"""
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Aggregate", executionPlan)
        self.env.assertNotIn("Distinct", executionPlan)

        resultset = graph.query(query).result_set
        expected = [['Ailon', 3],
                    ['Alon', 3],
                    ['Boaz', 3],
                    ['Roi', 3]]
        self.env.assertEqual(resultset, expected)


        # Verify that the Distinct operation is not removed from a valid projection.
        query = """MATCH (src:person) WITH DISTINCT src MATCH (src)-[:know]->(dest) RETURN src.name, COUNT(dest) ORDER BY src.name"""
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Aggregate", executionPlan)
        self.env.assertIn("Distinct", executionPlan)

        resultset = graph.query(query).result_set
        # This query should emit the same result.
        self.env.assertEqual(resultset, expected)


