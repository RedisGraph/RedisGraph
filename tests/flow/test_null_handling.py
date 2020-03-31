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
        redis_graph = Graph("null_handling", redis_con)
        self.populate_graph()

    def populate_graph(self):
        # Create a single node.
        node = Node(label="L", properties={"v": "v1"})
        redis_graph.add_node(node)
        redis_graph.flush()

    # Error when attempting to create a relationship with a null endpoint.
    def test01_create_null(self):
        try:
            query = """MATCH (a) OPTIONAL MATCH (a)-[nonexistent_edge]->(nonexistent_node) CREATE (nonexistent_node)-[:E]->(a)"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

        try:
            query = """MATCH (a) OPTIONAL MATCH (a)-[nonexistent_edge]->(nonexistent_node) CREATE (a)-[:E]->(nonexistent_node)"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

    # Error when attempting to merge a relationship with a null endpoint.
    def test02_merge_null(self):
        try:
            query = """MATCH (a) OPTIONAL MATCH (a)-[nonexistent_edge]->(nonexistent_node) MERGE (nonexistent_node)-[:E]->(a)"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

        try:
            query = """MATCH (a) OPTIONAL MATCH (a)-[nonexistent_edge]->(nonexistent_node) MERGE (a)-[:E]->(nonexistent_node)"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

    # SET should update attributes on non-null entities and ignore null entities.
    def test03_set_null(self):
        query = """MATCH (a) OPTIONAL MATCH (a)-[nonexistent_edge]->(nonexistent_node) SET a.v2 = true, nonexistent_node.v2 = true RETURN a.v2, nonexistent_node.v2"""
        actual_result = redis_graph.query(query)
        # The property should be set on the real node and ignored on the null entity.
        assert(actual_result.properties_set == 1)
        expected_result = [[True, None]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # DELETE should ignore null entities.
    def test04_delete_null(self):
        query = """MATCH (a) OPTIONAL MATCH (a)-[nonexistent_edge]->(nonexistent_node) DELETE nonexistent_node"""
        actual_result = redis_graph.query(query)
        assert(actual_result.nodes_deleted == 0)

    # Functions should handle null inputs appropriately.
    def test05_null_function_inputs(self):
        query = """MATCH (a) OPTIONAL MATCH (a)-[r]->(b) RETURN type(r), labels(b), b.v * 5"""
        actual_result = redis_graph.query(query)
        expected_result = [[None, None, None]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Path functions should handle null inputs appropriately.
    def test06_null_named_path_function_inputs(self):
        query = """MATCH (a) OPTIONAL MATCH p = (a)-[r]->() RETURN p, length(p), collect(relationships(p))"""
        actual_result = redis_graph.query(query)
        # The path and function calls on it should return NULL, while collect() returns an empty array.
        expected_result = [[None, None, []]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # List functions should handle null inputs appropriately.
    def test07_null_list_function_inputs(self):
        expected_result = [[None]]

        # NULL list argument to subscript.
        query = """WITH NULL as list RETURN list[0]"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        # NULL list argument to slice.
        query = """WITH NULL as list RETURN list[0..5]"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        # NULL list argument to HEAD.
        query = """WITH NULL as list RETURN head(list)"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        # NULL list argument to TAIL.
        query = """WITH NULL as list RETURN tail(list)"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        # NULL list argument to IN.
        query = """WITH NULL as list RETURN 'val' in list"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        # NULL list argument to SIZE.
        query = """WITH NULL as list RETURN size(list)"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        # NULL subscript argument.
        query = """WITH ['a'] as list RETURN list[NULL]"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        # NULL IN non-empty list should return NULL.
        query = """RETURN NULL in ['val']"""
        actual_result = redis_graph.query(query)
        expected_result = [[None]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # NULL arguments to slice.
        query = """WITH ['a'] as list RETURN list[0..NULL]"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        # NULL range argument should produce an error.
        query = """RETURN range(NULL, 5)"""
        try:
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

        # NULL IN empty list should return false.
        query = """RETURN NULL in []"""
        actual_result = redis_graph.query(query)
        expected_result = [[False]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Aggregate functions should handle null inputs appropriately.
    def test08_aggregate_function_inputs(self):
        # SUM should sum all non-null inputs.
        query = """UNWIND [1, NULL, 3] AS a RETURN sum(a)"""
        actual_result = redis_graph.query(query)
        expected_result = [[4]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # SUM should return 0 given a fully NULL input.
        query = """WITH NULL AS a RETURN sum(a)"""
        actual_result = redis_graph.query(query)
        expected_result = [[0]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # COUNT should count all non-null inputs.
        query = """UNWIND [1, NULL, 3] AS a RETURN count(a)"""
        actual_result = redis_graph.query(query)
        expected_result = [[2]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # COUNT should return 0 given a fully NULL input.
        query = """WITH NULL AS a RETURN count(a)"""
        actual_result = redis_graph.query(query)
        expected_result = [[0]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # COLLECT should ignore null inputs.
        query = """UNWIND [1, NULL, 3] AS a RETURN collect(a)"""
        actual_result = redis_graph.query(query)
        expected_result = [[[1, 3]]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # COLLECT should return an empty array on all null inputs.
        query = """WITH NULL AS a RETURN collect(a)"""
        actual_result = redis_graph.query(query)
        expected_result = [[[]]]
        self.env.assertEquals(actual_result.result_set, expected_result)
