import redis
from RLTest import Env
from base import FlowTestsBase
from redisgraph import Graph, Node, Edge

redis_graph = None
GRAPH_ID = "map_test"

class testMap(FlowTestsBase):
    def __init__(self):
        global redis_graph
        self.env = Env(decodeResponses=True)
        redis_con = self.env.getConnection()
        redis_graph = Graph(GRAPH_ID, redis_con)
        self.populate_graph()

    def populate_graph(self):
        # Construct a graph with the form:
        # (v1)-[:E]->(v2)-[:E]->(v3)
        q = """CREATE (:L {val:1})-[:E]->(:L {val:2})-[:E]->(:L {val:3})"""
        redis_graph.query(q)

    # Validate basic map lookup operations
    def test01_basic_map_accesses(self):
        # Return a full map
        query = """WITH {val: 5} AS map RETURN map"""
        query_result = redis_graph.query(query)
        expected_result = [[{'val': 5}]]
        self.env.assertEquals(query_result.result_set, expected_result)

        # Return a map value with dot notation
        query = """WITH {val: 5} AS map RETURN map.val"""
        query_result = redis_graph.query(query)
        expected_result = [[5]]
        self.env.assertEquals(query_result.result_set, expected_result)

        # Return a map value with bracket notation
        query = """WITH {val: 5} AS map RETURN map['val']"""
        query_result = redis_graph.query(query)
        expected_result = [[5]]
        self.env.assertEquals(query_result.result_set, expected_result)

    # Validate map projection behavior
    def test02_map_projections(self):
        query = """MATCH (a) RETURN a {.val} ORDER BY a.val"""
        query_result = redis_graph.query(query)
        expected_result = [[{'val': 1}],
                           [{'val': 2}],
                           [{'val': 3}]]
        self.env.assertEquals(query_result.result_set, expected_result)

        query = """WITH 'lit' AS literal MATCH (a) RETURN a {.val, literal} ORDER BY a.val"""
        query_result = redis_graph.query(query)
        expected_result = [[{'val': 1, 'literal': 'lit'}],
                           [{'val': 2, 'literal': 'lit'}],
                           [{'val': 3, 'literal': 'lit'}]]
        self.env.assertEquals(query_result.result_set, expected_result)

    # Validate behaviors of nested maps
    def test03_nested_maps(self):
        # Return a map with nesting
        query = """WITH {val: 5, nested: {nested_val: 'nested_str'}} AS map RETURN map"""
        query_result = redis_graph.query(query)
        expected_result = [[{'val': 5, 'nested': {'nested_val': 'nested_str'}}]]
        self.env.assertEquals(query_result.result_set, expected_result)

        # Return just the nested value
        query = """WITH {val: 5, nested: {nested_val: 'nested_str'}} AS map RETURN map.nested"""
        query_result = redis_graph.query(query)
        expected_result = [[{'nested_val': 'nested_str'}]]
        self.env.assertEquals(query_result.result_set, expected_result)

        # Perform operations on map values
        query = """WITH {val: 5, nested: {nested_val: 'nested_str'}} AS map RETURN map.val + '_' + map.nested.nested_val"""
        query_result = redis_graph.query(query)
        expected_result = [['5_nested_str']]
        self.env.assertEquals(query_result.result_set, expected_result)

        # Deeply nested map
        query = """RETURN {a: {b: {c: {d: {e: {f: {g: {h: {i: {j: {}}}}}}}}}}}"""
        query_result = redis_graph.query(query)
        expected_result = [[{'a': {'b': {'c': {'d': {'e': {'f': {'g': {'h': {'i': {'j': {}}}}}}}}}}}]]
        self.env.assertEquals(query_result.result_set, expected_result)

    # Validate map sorting logic (first by keys, then by values)
    def test04_map_sorting(self):
        query = """UNWIND[{b: 1}, {a: 2}] AS map RETURN map ORDER BY map"""
        query_result = redis_graph.query(query)
        expected_result = [[{'a': 2}],
                           [{'b': 1}]]
        self.env.assertEquals(query_result.result_set, expected_result)

        query = """UNWIND[{a: 2}, {a: 1}] AS map RETURN map ORDER BY map"""
        query_result = redis_graph.query(query)
        expected_result = [[{'a': 1}],
                           [{'a': 2}]]
        self.env.assertEquals(query_result.result_set, expected_result)

        query = """UNWIND[{a: 2}, {x: 1, k: 2}] AS map RETURN map ORDER BY map"""
        query_result = redis_graph.query(query)
        expected_result = [[{'a': 2}],
                           [{'x': 1, 'k': 2}]]
        self.env.assertEquals(query_result.result_set, expected_result)

    # Validate map comparison logic (first by keys, then by values)
    def test05_map_comparison(self):
        query = """WITH {b: 2} AS map_1, {a: 1} AS map_2 RETURN map_1 > map_2, map_1 < map_2, map_1 = map_2, map_1 <> map_2"""
        query_result = redis_graph.query(query)
        expected_result = [[True, False, False, True]]
        self.env.assertEquals(query_result.result_set, expected_result)

        query = """WITH {a: 2} AS map_1, {a: 1} AS map_2 RETURN map_1 > map_2, map_1 < map_2, map_1 = map_2, map_1 <> map_2"""
        query_result = redis_graph.query(query)
        expected_result = [[True, False, False, True]]
        self.env.assertEquals(query_result.result_set, expected_result)

        # Map equality is not predicated on key order.
        query = """WITH {a: 1, b: 2} AS map_1, {b: 2, a: 1} AS map_2 RETURN map_1 > map_2, map_1 < map_2, map_1 = map_2, map_1 <> map_2"""
        query_result = redis_graph.query(query)
        expected_result = [[False, False, True, False]]
        self.env.assertEquals(query_result.result_set, expected_result)

    # Validate that maps are handled correctly by the DISTINCT operator.
    def test05_map_distinct(self):
        # Map uniqueness is not predicated on key order.
        query = """UNWIND[{b: 2, a: 1}, {b: 2, a: 1}, {a: 1, b: 2}] AS map RETURN DISTINCT map"""
        query_result = redis_graph.query(query)
        expected_result = [[{'a': 1, 'b': 2}]]
        self.env.assertEquals(query_result.result_set, expected_result)

    # Validate that trying to access a map with a non-string key errors gracefully.
    def test06_map_invalid_key_lookup(self):
        try:
            query = """WITH {val: 5} AS map RETURN map[0]"""
            redis_graph.query(query)
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertIn("Type mismatch", str(e))

