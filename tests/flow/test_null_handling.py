import os
import sys
import redis
from RLTest import Env
from redisgraph import Graph, Node, Edge
from base import FlowTestsBase

redis_graph = None

class testNullHandlingFlow(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph("G", redis_con)
        self.populate_graph()

    def populate_graph(self):
        # Create a single node.
        node = Node(label="L", properties={"v": "v1"})
        redis_graph.add_node(node)
        redis_graph.commit()

    # Error when attempting to create a relationship with a null endpoint.
    def test01_create_null(self):
        try:
            query = """MATCH (a) OPTIONAL MATCH (a)-[nonexistent_edge]->(nonexistent_node) CREATE (nonexistent_node)-[:E]->(b)"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

    # SET should update attributes on non-null entities and ignore null entities.
    def test02_set_null(self):
        query = """MATCH (a) OPTIONAL MATCH (a)-[nonexistent_edge]->(nonexistent_node) SET a.v2 = true, nonexistent_node.v2 = true RETURN a.v2, nonexistent_node.v2"""
        actual_result = redis_graph.query(query)
        # The property should be set on the real node and ignored on the null entity.
        assert(actual_result.properties_set == 1)
        expected_result = [[True, None]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # DELETE should ignore null entities.
    def test03_delete_null(self):
        query = """MATCH (a) OPTIONAL MATCH (a)-[nonexistent_edge]->(nonexistent_node) DELETE nonexistent_node"""
        actual_result = redis_graph.query(query)
        # The property should be set on the real node and ignored on the null entity.
        assert(actual_result.nodes_deleted == 0)

    # Functions should handle null inputs appropriately.
    def test04_null_function_inputs(self):
        query = """MATCH (a) OPTIONAL MATCH (a)-[r]->(b) RETURN type(r), labels(b), b.v * 5"""
        actual_result = redis_graph.query(query)
        expected_result = [[None, None, None]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Path functions should handle null inputs appropriately.
    def test05_null_named_path_function_inputs(self):
        query = """MATCH (a) OPTIONAL MATCH p = (a)-[r]->() RETURN p, length(p), collect(relationships(p))"""
        actual_result = redis_graph.query(query)
        # The path and function calls on it should return NULL, while collect() returns an empty array.
        expected_result = [[None, None, []]]
        self.env.assertEquals(actual_result.result_set, expected_result)
