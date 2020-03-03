import os
import sys
from RLTest import Env
from redisgraph import Graph, Node, Edge

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from base import FlowTestsBase

redis_graph = None

class testBoundVariables(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph("G", redis_con)
        self.populate_graph()

    def populate_graph(self):
        global redis_graph

        # Construct a graph with the form:
        # (v1)-[:E]->(v2)-[:E]->(v3)
        node_props = ['v1', 'v2', 'v3']

        nodes = []
        for idx, v in enumerate(node_props):
            node = Node(label="L", properties={"val": v})
            nodes.append(node)
            redis_graph.add_node(node)

        edge = Edge(nodes[0], "E", nodes[1])
        redis_graph.add_edge(edge)

        edge = Edge(nodes[1], "E", nodes[2])
        redis_graph.add_edge(edge)

        redis_graph.commit()

    def test01_with_projected_entity(self):
        query = """MATCH (a:L {val: 'v1'}) WITH a MATCH (a)-[e]->(b) RETURN b.val"""
        actual_result = redis_graph.query(query)

        # Verify that this query does not generate a Cartesian product.
        execution_plan = redis_graph.execution_plan(query)
        self.env.assertNotIn('Cartesian Product', execution_plan)

        # Verify results.
        expected_result = [['v2']]
        self.env.assertEquals(actual_result.result_set, expected_result)

    def test02_match_create_bound_variable(self):
        # Extend the graph such that the new form is:
        # (v1)-[:E]->(v2)-[:E]->(v3)-[:e]->(v4)
        query = """MATCH (a:L {val: 'v3'}) CREATE (a)-[:E]->(b:L {val: 'v4'}) RETURN b.val"""
        actual_result = redis_graph.query(query)
        expected_result = [['v4']]
        self.env.assertEquals(actual_result.result_set, expected_result)
        self.env.assertEquals(actual_result.relationships_created, 1)
        self.env.assertEquals(actual_result.nodes_created, 1)

    def test03_procedure_match_bound_variable(self):
        # Create a full-text index.
        redis_graph.call_procedure("db.idx.fulltext.createNodeIndex", 'L', 'val')

        # Project the result of scanning this index into a MATCH pattern.
        query = """CALL db.idx.fulltext.queryNodes('L', 'v1') YIELD node MATCH (node)-[]->(b) RETURN b.val
"""
        # Verify that execution begins at the procedure call and proceeds into the traversals.
        execution_plan = redis_graph.execution_plan(query)
        # For the moment, we'll just verify that ProcedureCall appears later in the plan than
        # its parent, Conditional Traverse.
        traverse_idx = execution_plan.index("Conditional Traverse")
        call_idx = execution_plan.index("ProcedureCall")
        self.env.assertTrue(call_idx > traverse_idx)

        # Verify the results
        actual_result = redis_graph.query(query)
        expected_result = [['v2']]
        self.env.assertEquals(actual_result.result_set, expected_result)
