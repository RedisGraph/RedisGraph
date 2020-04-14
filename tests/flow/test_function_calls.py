import redis
import os
import sys
from RLTest import Env
from base import FlowTestsBase
from redisgraph import Graph, Node, Edge


graph = None
redis_con = None
people = ["Roi", "Alon", "Ailon", "Boaz"]

class testFunctionCallsFlow(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        global graph
        global redis_con
        redis_con = self.env.getConnection()
        graph = Graph("G", redis_con)
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

    def expect_type_error(self, query):
        try:
            graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting a type error.
            self.env.assertIn("Type mismatch", e.message)
    
    def expect_error(self, query, expected_err_msg):
        try:
            graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting a type error.
            self.env.assertIn(expected_err_msg, e.message)

    # Validate capturing of errors prior to query execution.
    def test01_compile_time_errors(self):
        query = """RETURN toUpper(5)"""
        self.expect_type_error(query)

        query = """RETURN 'a' * 2"""
        self.expect_type_error(query)

        query = """RETURN max(1 + min(2))"""
        self.expect_error(query, "Can't use aggregate functions inside of aggregate functions")

    def test02_boolean_comparisons(self):
        query = """RETURN true = 5"""
        actual_result = graph.query(query)
        expected_result = [[False]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """RETURN true <> 'str'"""
        actual_result = graph.query(query)
        expected_result = [[True]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """RETURN 'anything' <> NULL"""
        actual_result = graph.query(query)
        expected_result = [[None]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """RETURN 'anything' = NULL"""
        actual_result = graph.query(query)
        expected_result = [[None]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """RETURN 10 >= 1.5"""
        actual_result = graph.query(query)
        expected_result = [[True]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """RETURN -1 < 1"""
        actual_result = graph.query(query)
        expected_result = [[True]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    def test03_boolean_errors(self):
        query = """RETURN 'str' < 5.5"""
        self.expect_type_error(query)

        query = """RETURN true > 5"""
        self.expect_type_error(query)

        query = """MATCH (a) RETURN a < 'anything' LIMIT 1"""
        self.expect_type_error(query)

    def test04_entity_functions(self):
        query = "RETURN ID(5)"
        self.expect_type_error(query)

        query = "MATCH (a) RETURN ID(a) ORDER BY ID(a) LIMIT 3"
        actual_result = graph.query(query)
        expected_result = [[0], [1], [2]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = "MATCH (a)-[e]->() RETURN ID(e) ORDER BY ID(e) LIMIT 3"
        actual_result = graph.query(query)
        expected_result = [[0], [1], [2]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = "RETURN EXISTS(null)"
        actual_result = graph.query(query)
        expected_result = [[False]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = "RETURN EXISTS('anything')"
        actual_result = graph.query(query)
        expected_result = [[True]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    def test07_nonmap_errors(self):
        query = """MATCH (a) WITH a.name AS scalar RETURN scalar.name"""
        self.expect_type_error(query)

    def test08_apply_all_function(self):
        query = "MATCH () RETURN COUNT(*)"
        actual_result = graph.query(query)
        expected_result = [[4]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = "UNWIND [1, 2] AS a RETURN COUNT(*)"
        actual_result = graph.query(query)
        expected_result = [[2]]
        self.env.assertEquals(actual_result.result_set, expected_result)
    
    def test09_static_aggregation(self):
        query = "RETURN count(*)"
        actual_result = graph.query(query)
        expected_result = [[1]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = "RETURN max(2)"
        actual_result = graph.query(query)
        expected_result = [[2]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = "RETURN min(3)"
        actual_result = graph.query(query)
        expected_result = [[3]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    def test10_modulo_inputs(self):
        # Validate modulo with integer inputs.
        query = "RETURN 5 % 2"
        actual_result = graph.query(query)
        expected_result = [[1]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Validate modulo with a floating-point dividend.
        query = "RETURN 5.5 % 2"
        actual_result = graph.query(query)
        expected_result = [[1.5]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Validate modulo with a floating-point divisor.
        query = "RETURN 5 % 2.5"
        actual_result = graph.query(query)
        expected_result = [[0]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Validate modulo with both a floating-point dividen and a floating-point divisor.
        query = "RETURN 5.5 % 2.5"
        actual_result = graph.query(query)
        expected_result = [[0.5]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Validate modulo with negative integer inputs.
        query = "RETURN -5 % -2"
        actual_result = graph.query(query)
        expected_result = [[-1]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Validate modulo with negative floating-point inputs.
        query = "RETURN -5.5 % -2.5"
        actual_result = graph.query(query)
        expected_result = [[-0.5]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Aggregate functions should handle null inputs appropriately.
    def test11_null_aggregate_function_inputs(self):
        # SUM should sum all non-null inputs.
        query = """UNWIND [1, NULL, 3] AS a RETURN sum(a)"""
        actual_result = graph.query(query)
        expected_result = [[4]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # SUM should return 0 given a fully NULL input.
        query = """WITH NULL AS a RETURN sum(a)"""
        actual_result = graph.query(query)
        expected_result = [[0]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # COUNT should count all non-null inputs.
        query = """UNWIND [1, NULL, 3] AS a RETURN count(a)"""
        actual_result = graph.query(query)
        expected_result = [[2]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # COUNT should return 0 given a fully NULL input.
        query = """WITH NULL AS a RETURN count(a)"""
        actual_result = graph.query(query)
        expected_result = [[0]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # COLLECT should ignore null inputs.
        query = """UNWIND [1, NULL, 3] AS a RETURN collect(a)"""
        actual_result = graph.query(query)
        expected_result = [[[1, 3]]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # COLLECT should return an empty array on all null inputs.
        query = """WITH NULL AS a RETURN collect(a)"""
        actual_result = graph.query(query)
        expected_result = [[[]]]
        self.env.assertEquals(actual_result.result_set, expected_result)
