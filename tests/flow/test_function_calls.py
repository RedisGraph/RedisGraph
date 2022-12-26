from common import *
import math
import json

graph = None
redis_con = None
people = ["Roi", "Alon", "Ailon", "Boaz"]

class testFunctionCallsFlow(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global graph
        global redis_con
        redis_con = self.env.getConnection()
        graph = Graph(redis_con, "G")
        self.populate_graph()

    def populate_graph(self):
        global graph
        nodes = {}
        # Create entities
        for idx, p in enumerate(people):
            if idx % 2 == 0:
                labels = ["person"]
            else:
                labels = ["person", "student"]
            node = Node(label=labels, properties={"name": p, "val": idx})
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
            self.env.assertIn("Type mismatch", str(e))

    def expect_error(self, query, expected_err_msg):
        try:
            graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting a type error.
            self.env.assertIn(expected_err_msg, str(e))
    
    def get_res_and_assertEquals(self, query, expected_result):
        actual_res = graph.query(query)
        self.env.assertEquals(actual_res.result_set, expected_result)

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

        # COLLECT should associate false and 'false' to different groups.
        query = "UNWIND [false,'false',0,'0'] AS a RETURN a, count(a)"
        actual_result = graph.query(query)
        expected_result = [[0, 1], [False, 1], ["false", 1], ['0', 1]]
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

        query = "RETURN max([2])"
        actual_result = graph.query(query)
        expected_result = [[[2]]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = "RETURN min(3)"
        actual_result = graph.query(query)
        expected_result = [[3]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = "RETURN min([3])"
        actual_result = graph.query(query)
        expected_result = [[[3]]]
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

        # Validate modulo on edge case -LONG_MIN%-1.
        # https://gcc.gnu.org/bugzilla/show_bug.cgi?id=30484
        query = "RETURN toInteger(1.2289948315394e+19) % -1"
        actual_result = graph.query(query)
        expected_result = [[0]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Validate modulo by 0
        query = "RETURN 3 % 0"
        try:
            actual_result = graph.query(query)
        except redis.ResponseError as e:
            self.env.assertContains("Division by zero", str(e))

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

    # Verify that nested functions that perform heap allocations return properly.
    def test12_nested_heap_functions(self):
        query = """MATCH p = (n) WITH head(nodes(p)) AS node RETURN node.name ORDER BY node.name"""
        actual_result = graph.query(query)
        expected_result = [['Ailon'],
                           ['Alon'],
                           ['Boaz'],
                           ['Roi']]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # CASE...WHEN statements should properly handle NULL, false, and true evaluations.
    def test13_case_when_inputs(self):
        # Simple case form: single value evaluation.
        query = """UNWIND [NULL, true, false] AS v RETURN v, CASE v WHEN true THEN v END"""
        actual_result = graph.query(query)
        expected_result = [[None, None],
                           [True, True],
                           [False, None]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """UNWIND [NULL, true, false] AS v RETURN v, CASE v WHEN true THEN v WHEN false THEN v END"""
        actual_result = graph.query(query)
        expected_result = [[None, None],
                           [True, True],
                           [False, False]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Generic case form: evaluation for each case.
        query = """UNWIND [NULL, true, false] AS v RETURN v, CASE WHEN v THEN v END"""
        actual_result = graph.query(query)
        # Only the true value should return non-NULL.
        expected_result = [[None, None],
                           [True, True],
                           [False, None]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """UNWIND [NULL, true, false] AS v RETURN v, CASE WHEN v IS NOT NULL THEN v END"""
        actual_result = graph.query(query)
        # The true and false values should both return non-NULL.
        expected_result = [[None, None],
                           [True, True],
                           [False, False]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # CASE...WHEN statements should manage allocated values properly.
    def test14_case_when_memory_management(self):
        # Simple case form: single value evaluation.
        query = """WITH 'A' AS a WITH CASE a WHEN 'A' THEN toString(a) END AS key RETURN toLower(key)"""
        actual_result = graph.query(query)
        expected_result = [['a']]
        self.env.assertEquals(actual_result.result_set, expected_result)
        # Generic case form: evaluation for each case.
        query = """WITH 'A' AS a WITH CASE WHEN true THEN toString(a) END AS key RETURN toLower(key)"""
        actual_result = graph.query(query)
        expected_result = [['a']]
        self.env.assertEquals(actual_result.result_set, expected_result)

    def test15_aggregate_error_handling(self):
        functions = ["avg",
                     "collect",
                     "count",
                     "max",
                     "min",
                     "sum",
                     "percentileDisc",
                     "percentileCont",
                     "stDev"]
        # Test all functions for invalid argument counts.
        for function in functions:
            query = """UNWIND range(0, 10) AS val RETURN %s(val, val, val)""" % (function)
            self.expect_error(query, "Received 3 arguments")

        # Test numeric functions for invalid input types.
        numeric_functions = ["avg",
                             "sum",
                             "stDev"]
        for function in numeric_functions:
            query = """UNWIND ['a', 'b', 'c'] AS val RETURN %s(val)""" % (function)
            self.expect_type_error(query)

        # Test invalid numeric input for percentile function.
        query = """UNWIND range(0, 10) AS val RETURN percentileDisc(val, -1)"""
        self.expect_error(query, "must be a number in the range 0.0 to 1.0")

    # startNode and endNode calls should return the appropriate nodes.
    def test16_edge_endpoints(self):
        query = """MATCH (a)-[e]->(b) RETURN a.name, startNode(e).name, b.name, endNode(e).name"""
        actual_result = graph.query(query)
        for row in actual_result.result_set:
            self.env.assertEquals(row[0], row[1])
            self.env.assertEquals(row[2], row[3])

    def test17_to_json(self):
        # Test JSON literal values in an array.
        query = """RETURN toJSON([1, 0.000000000000001, 'str', true, NULL])"""
        actual_result = graph.query(query)
        parsed = json.loads(actual_result.result_set[0][0])
        self.env.assertEquals(parsed, [1, 0.000000000000001, "str", True, None])

        # Test JSON an empty array value.
        query = """WITH [] AS arr RETURN toJSON(arr)"""
        actual_result = graph.query(query)
        parsed = json.loads(actual_result.result_set[0][0])
        self.env.assertEquals(parsed, [])

        # Test JSON an empty map value.
        query = """WITH {} AS map RETURN toJSON(map)"""
        actual_result = graph.query(query)
        parsed = json.loads(actual_result.result_set[0][0])
        self.env.assertEquals(parsed, {})

        # Test converting a map projection.
        query = """MATCH (n {val: 1}) RETURN toJSON(n {.val, .name})"""
        actual_result = graph.query(query)
        parsed = json.loads(actual_result.result_set[0][0])
        self.env.assertEquals(parsed, {"name": "Alon", "val": 1})

        # Test converting a full node.
        query = """MATCH (n {val: 1}) RETURN toJSON(n)"""
        actual_result = graph.query(query)
        parsed = json.loads(actual_result.result_set[0][0])
        self.env.assertEquals(parsed, {"type": "node", "id": 1, "labels": ["person", "student"], "properties": {"name": "Alon", "val": 1}})

        # Test converting a full edge.
        query = """MATCH ({val: 0})-[e:works_with]->({val: 1}) RETURN toJSON(e)"""
        actual_result = graph.query(query)
        start = {"id": 0, "labels": ["person"], "properties": {"name": "Roi", "val": 0}}
        end = {"id": 1, "labels": ["person", "student"], "properties": {"name": "Alon", "val": 1}}
        parsed = json.loads(actual_result.result_set[0][0])
        self.env.assertEquals(parsed, {"type": "relationship", "id": 12, "relationship": "works_with", "properties": {}, "start": start, "end": end})

        # Test converting a path.
        query = """MATCH path=({val: 0})-[e:works_with]->({val: 1}) RETURN toJSON(path)"""
        actual_result = graph.query(query)
        expected = [{'type': 'node', 'id': 0, 'labels': ['person'], 'properties': {'name': 'Roi', 'val': 0}}, {'type': 'relationship', 'id': 12, 'relationship': 'works_with', 'properties': {}, 'start': {'id': 0, 'labels': ['person'], 'properties': {'name': 'Roi', 'val': 0}}, 'end': {'id': 1, 'labels': ['person', 'student'], 'properties': {'name': 'Alon', 'val': 1}}}, {'type': 'node', 'id': 1, 'labels': ['person', 'student'], 'properties': {'name': 'Alon', 'val': 1}}]
        parsed = json.loads(actual_result.result_set[0][0])
        self.env.assertEquals(parsed, expected)

        # Test JSON literal values in an point.
        query = """RETURN toJSON(point({ longitude: 167.697555, latitude: 0.402313 }))"""
        actual_result = graph.query(query)
        parsed = json.loads(actual_result.result_set[0][0])
        self.env.assertEquals(parsed, {"crs": "wgs-84", "latitude": 0.402313, "longitude": 167.697556, "height": None})

    # Memory should be freed properly when the key values are heap-allocated.
    def test18_allocated_keys(self):
        query = """UNWIND ['str1', 'str1', 'str2', 'str1'] AS key UNWIND [1, 2, 3] as agg RETURN toUpper(key) AS key, collect(DISTINCT agg) ORDER BY key"""
        actual_result = graph.query(query)
        expected_result = [['STR1', [1, 2, 3]],
                           ['STR2', [1, 2, 3]]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    def test19_has_labels(self):
        # Test existing label
        query = """MATCH (n) WHERE n:person RETURN n.name"""
        actual_result = graph.query(query)
        expected_result = [['Roi'], ['Alon'], ['Ailon'], ['Boaz']]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test not existing label
        query = """MATCH (n) WHERE n:L RETURN n.name"""
        actual_result = graph.query(query)
        expected_result = []
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test multi label
        query = """MATCH (n) WHERE n:person:L RETURN n.name"""
        actual_result = graph.query(query)
        expected_result = []
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test multi label
        query = """MATCH (n) WHERE n:person:student RETURN n.name"""
        actual_result = graph.query(query)
        expected_result = [['Alon'], ['Boaz']]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test or between different labels label
        query = """MATCH (n) WHERE n:person OR n:L RETURN n.name"""
        actual_result = graph.query(query)
        expected_result = [['Roi'], ['Alon'], ['Ailon'], ['Boaz']]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test multi label using functions
        query = """MATCH (n) WHERE hasLabels(n, ['person', 'L']) RETURN n.name"""
        actual_result = graph.query(query)
        expected_result = []
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test multi label using functions
        query = """MATCH (n) WHERE hasLabels(n, ['person', 'student']) RETURN n.name"""
        actual_result = graph.query(query)
        expected_result =  [['Alon'], ['Boaz']]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test has labels using functions mismatch type
        query = """MATCH (n) WHERE hasLabels(n, ['person', 1]) RETURN n.name"""
        try:
            graph.query(query)
        except redis.ResponseError as e:
            self.env.assertContains("Type mismatch: expected String but was Integer", str(e))

    def test20_keys(self):
        # Test retrieving keys of a nested map
        query = """RETURN keys({a: 5, b: 10})"""
        actual_result = graph.query(query)
        expected_result = [[['a', 'b']]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test retrieving keys of a map reference
        query = """WITH {a: 5, b: 10} AS map RETURN keys(map)"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test retrieving keys of a node
        query = """MATCH (n:person {name: 'Roi'}) RETURN keys(n)"""
        actual_result = graph.query(query)
        expected_result = [[['name', 'val']]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test retrieving keys of an (empty) edge
        query = """MATCH (:person {name: 'Roi'})-[e:works_with]->(:person {name: 'Alon'}) RETURN keys(e)"""
        actual_result = graph.query(query)
        expected_result = [[[]]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test a null input
        query = """WITH NULL AS map RETURN keys(map)"""
        actual_result = graph.query(query)
        expected_result = [[None]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test trying to retrieve keys of an invalid type
        query = """WITH 10 AS map RETURN keys(map)"""
        self.expect_type_error(query)

    def test21_distinct_memory_management(self):
        # validate behavior of the DISTINCT function with allocated values
        query = """MATCH (a {val: 0}) RETURN collect(DISTINCT a { .name })"""
        actual_result = graph.query(query)
        expected_result = [[[{'name': 'Roi'}]]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    def test22_large_list_argument(self):
        # validate that large lists arguments are not allocated on stack
        large_list = str([1] * 1000000)
        query = f"""RETURN {large_list}"""
        actual_result = graph.query(query)
        self.env.assertEquals(len(actual_result.result_set[0][0]), 1000000)
    
    def test23_toInteger(self):
        # expect calling toInteger to succeed
        queries = [
            """RETURN toInteger(1)""",
            """RETURN toInteger(1.1)""",
            """RETURN toInteger(1.9)""",
            """RETURN toInteger('1')""",
            """RETURN toInteger('1.1')""",
            """RETURN toInteger('1.9')"""
        ]
        for query in queries:
            actual_result = graph.query(query)
            self.env.assertEquals(actual_result.result_set[0][0], 1)

        # expect calling toInteger to return NULL
        queries = [
            """RETURN toInteger('z')""",
            """RETURN toInteger(NULL)""",
            """RETURN toInteger('')"""
        ]
        for query in queries:
            actual_result = graph.query(query)
            self.env.assertEquals(actual_result.result_set[0][0], None)

    def test24_substring(self):
        query = """RETURN SUBSTRING('muchacho', 0, 4)"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], "much")

        query = """RETURN SUBSTRING('muchacho', 3, 20)"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], "hacho")

        query = """RETURN SUBSTRING(NULL, 3, 20)"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], None)

        # the requested length is too long and overflowing
        query = """RETURN SUBSTRING('ab', 1, 999999999999999999999999999999999999999999999)"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], "b")

        try:
            query = """RETURN SUBSTRING("muchacho", 3, -20)"""
            graph.query(query)
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertEqual(str(e), "length must be positive integer")

        try:
            query = """RETURN SUBSTRING("muchacho", -3, 3)"""
            graph.query(query)
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertEqual(str(e), "start must be positive integer")

    def test25_left(self):
        query = """RETURN LEFT('muchacho', 4)"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], "much")

        query = """RETURN LEFT('muchacho', 100)"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], "muchacho")

        query = """RETURN LEFT(NULL, 100)"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], None)

        try:
            query = """RETURN LEFT('', -100)"""
            graph.query(query)
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertEqual(str(e), "length must be positive integer")

    def tes26_right(self):
        query = """RETURN RIGHT('muchacho', 4)"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], "acho")

        query = """RETURN RIGHT('muchacho', 100)"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], "muchacho")

        query = """RETURN RIGHT(NULL, 100)"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], None)

        try:
            query = """RETURN RIGHT('', -100)"""
            graph.query(query)
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertEqual(str(e), "length must be positive integer")

    def test27_string_concat(self):
        larg_double = 1.123456e300
        query = f"""RETURN '' + {larg_double} + {larg_double}"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], "%f%f" % (larg_double, larg_double))

    def test28_min_max(self):
        query = "UNWIND [[1], [2], [2], [1]] AS x RETURN max(x), min(x)"
        actual_result = graph.query(query)
        expected_result = [[[2], [1]]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = "UNWIND [1, 2, '1' ,'2' ,[1] ,[2] ,1 ,2, '1', '2', NULL, True] AS x RETURN max(x), min(x)"
        actual_result = graph.query(query)
        expected_result = [[2, [1]]]
        self.env.assertEquals(actual_result.result_set, expected_result)    

    def test29_Expression(self):
        query_to_expected_result = {
            "RETURN 'muchacho'": [['muchacho']],
            "RETURN 1": [[1]],
            "RETURN 1+2*3": [[7]],
            "RETURN 1 + 1 + 1 + 1 + 1 + 1": [[6]],
            "RETURN ABS(-5 + 2 * 1)": [[3]],
            "RETURN 'a' + 'b'": [['ab']],
            "RETURN 1 + 2 + 'a' + 2 + 1": [['3a21']],
            "RETURN 2 * 2 + 'a' + 3 * 3": [['4a9']],
            "RETURN 9 % 5": [[4]],
            "RETURN 9 % 5 % 3": [[1]]
        }
        for query, expected_result in query_to_expected_result.items():
            self.get_res_and_assertEquals(query, expected_result)
        
    def test30_NullArithmetic(self):
        query_to_expected_result = {
            "RETURN null + 1": [[None]],
            "RETURN 1 + null": [[None]],
            "RETURN null - 1": [[None]],
            "RETURN 1 - null": [[None]],
            "RETURN 1 * null": [[None]],
            "RETURN null / 1": [[None]],
            "RETURN 1 / null": [[None]],
            "RETURN 5 % null": [[None]],
            "RETURN null % 5": [[None]]
        }
        for query, expected_result in query_to_expected_result.items():
            self.get_res_and_assertEquals(query, expected_result)
    
    def test31_Abs(self):
        query_to_expected_result = {
            "RETURN ABS(1)": [[1]],
            "RETURN ABS(-1)": [[1]],
            "RETURN ABS(0)": [[0]],
            "RETURN ABS(NULL)": [[None]]
        }
        for query, expected_result in query_to_expected_result.items():
            self.get_res_and_assertEquals(query, expected_result)
    
    def test32_Aggregate(self):
        query_to_expected_result = {
            "UNWIND [1, 1, 1] AS one RETURN SUM(one)": [[3]],
            "UNWIND [1, 1, 1] AS one WITH SUM(one) AS s RETURN s+2": [[5]]
        }
        for query, expected_result in query_to_expected_result.items():
            self.get_res_and_assertEquals(query, expected_result)
    
    def test33_Ciel(self):
        query_to_expected_result = {
            "RETURN CEIL(0.5)": [[1]],
            "RETURN CEIL(1)": [[1]],
            "RETURN CEIL(0.1)": [[1]],
            "RETURN CEIL(NULL)": [[None]]
        }
        for query, expected_result in query_to_expected_result.items():
            self.get_res_and_assertEquals(query, expected_result)
    
    def test34_Floor(self):
        query_to_expected_result = {
            "RETURN FLOOR(0.5)": [[0]], 
            "RETURN FLOOR(1)": [[1]], 
            "RETURN FLOOR(0.1)": [[0]], 
            "RETURN FLOOR(NULL)": [[None]] 
        }
        for query, expected_result in query_to_expected_result.items():
            self.get_res_and_assertEquals(query, expected_result)
    
    def test35_Round(self):
        query_to_expected_result = {
            "RETURN ROUND(0)": [[0]], 
            "RETURN ROUND(0.49)": [[0]], 
            "RETURN ROUND(0.5)": [[1]], 
            "RETURN ROUND(1)": [[1]], 
            "RETURN ROUND(NULL)": [[None]]
        }
        for query, expected_result in query_to_expected_result.items():
            self.get_res_and_assertEquals(query, expected_result)

    def test36_Sign(self):
        query_to_expected_result = {
            "RETURN SIGN(0)": [[0]], 
            "RETURN SIGN(-1)": [[-1]], 
            "RETURN SIGN(1)": [[1]], 
            "RETURN SIGN(NULL)": [[None]]
        }
        for query, expected_result in query_to_expected_result.items():
            self.get_res_and_assertEquals(query, expected_result)

    def test37_Pow(self):
        query_to_expected_result = {
            "RETURN pow(1,0)": [[1]], 
            "RETURN 1^0": [[1]], 
            "RETURN pow(0,1)": [[0]], 
            "RETURN 0^1": [[0]], 
            "RETURN pow(0,0)": [[1]], 
            "RETURN 0^0": [[1]], 
            "RETURN pow(2,3)": [[8]], 
            "RETURN 2^3": [[8]], 
            "RETURN pow(2,-3)": [[0.125]], 
            "RETURN 2^-3": [[0.125]], 
            "RETURN 2^(-3)": [[0.125]], 
            "RETURN pow(0.5,2)": [[0.25]], 
            "RETURN 0.5^2": [[0.25]], 
            "RETURN pow(-1,2)": [[1]], 
            "RETURN -1^2": [[1]], 
            "RETURN (-1)^2": [[1]], 
            "RETURN pow(NULL,1)": [[None]], 
            "RETURN NULL^1": [[None]], 
            "RETURN pow(1,NULL)": [[None]], 
            "RETURN 1^NULL": [[None]], 
            "RETURN pow(NULL,NULL)": [[None]], 
            "RETURN NULL^NULL": [[None]]
        }
        for query, expected_result in query_to_expected_result.items():
            self.get_res_and_assertEquals(query, expected_result)
    
    def test38_Reverse(self):
        query_to_expected_result = {
            "RETURN REVERSE('muchacho')": [["ohcahcum"]], 
            "RETURN REVERSE('')": [[""]], 
            "RETURN REVERSE(NULL)": [[None]]
        }
        for query, expected_result in query_to_expected_result.items():
            self.get_res_and_assertEquals(query, expected_result)
    
    def test39_LTrim(self):
        query_to_expected_result = {
            "RETURN lTrim('   muchacho')": [["muchacho"]], 
            "RETURN lTrim('muchacho   ')": [["muchacho   "]], 
            "RETURN lTrim('   much   acho   ')": [["much   acho   "]], 
            "RETURN lTrim('muchacho')": [["muchacho"]], 
            "RETURN lTrim(NULL)": [[None]]
        }
        for query, expected_result in query_to_expected_result.items():
            self.get_res_and_assertEquals(query, expected_result)
    
    def test40_RTrim(self):
        query_to_expected_result = {
            "RETURN rTrim('   muchacho')": [["   muchacho"]], 
            "RETURN rTrim('muchacho   ')": [["muchacho"]], 
            "RETURN rTrim('   much   acho   ')": [["   much   acho"]], 
            "RETURN rTrim('muchacho')": [["muchacho"]], 
            "RETURN rTrim(NULL)": [[None]]
        }
        for query, expected_result in query_to_expected_result.items():
            self.get_res_and_assertEquals(query, expected_result)
    
    def test41_Trim(self):
        query_to_expected_result = {
            "RETURN trim('   muchacho')": [["muchacho"]],
            "RETURN trim('muchacho   ')": [["muchacho"]],
            "RETURN trim('   much   acho   ')": [["much   acho"]],
            "RETURN trim('muchacho')": [["muchacho"]],
            "RETURN trim(NULL)": [[None]]
        }
        for query, expected_result in query_to_expected_result.items():
            self.get_res_and_assertEquals(query, expected_result)
    
    def test42_ToLower(self):
        query_to_expected_result = {
            "RETURN toLower('MuChAcHo')": [['muchacho']],
            "RETURN toLower('mUcHaChO')": [['muchacho']],
            "RETURN toLower(NULL)": [[None]]
        }
        for query, expected_result in query_to_expected_result.items():
            self.get_res_and_assertEquals(query, expected_result)
    
    def test43_ToUpper(self):
        query_to_expected_result = {
            "RETURN toUpper('MuChAcHo')": [['MUCHACHO']],
            "RETURN toUpper('mUcHaChO')": [['MUCHACHO']],
            "RETURN toUpper(NULL)": [[None]]
        }
        for query, expected_result in query_to_expected_result.items():
            self.get_res_and_assertEquals(query, expected_result)
    
    def test44_Exists(self):
        query_to_expected_result = {
            "RETURN EXISTS(null)": [[0]],
            "RETURN EXISTS(1)": [[1]]
        }
        for query, expected_result in query_to_expected_result.items():
            self.get_res_and_assertEquals(query, expected_result)
    
    def test45_Case(self):
        query_to_expected_result = {
            "RETURN CASE 'brown' WHEN 'blue' THEN 1+0 WHEN 'brown' THEN 2-0 ELSE 3*1 END": [[2]],
            "RETURN CASE 'green' WHEN 'blue' THEN 1+0 WHEN 'brown' THEN 2-0 ELSE 3*1 END": [[3]],
            "RETURN CASE WHEN NULL THEN 1+0 WHEN true THEN 2-0 END": [[2]],
            "RETURN CASE WHEN NULL THEN 1+0 WHEN NULL THEN 2-0 ELSE 3*1 END": [[3]],
            "RETURN CASE WHEN NULL THEN 1+0 WHEN NULL THEN 2-0 END": [[None]],
            "RETURN CASE NULL WHEN NULL THEN NULL ELSE 'else' END AS result": [[None]],
            "RETURN CASE NULL WHEN 'value' THEN 'value' WHEN NULL THEN NULL ELSE 'else' END AS result": [[None]],
            "RETURN CASE NULL WHEN 'when' THEN 'then' ELSE NULL END AS result": [[None]],
            "RETURN CASE 'value' WHEN NULL THEN NULL ELSE true END AS result": [[True]],
            "RETURN CASE 'value' WHEN NULL THEN NULL WHEN 'value' THEN true ELSE false END AS result": [[True]]
        }
        for query, expected_result in query_to_expected_result.items():
            self.get_res_and_assertEquals(query, expected_result)
    
    def test46_AND(self):
        scenarios = [
            ['TRUE', 'FALSE', False],
            ['FALSE', 'TRUE', False],
            ['TRUE', 'TRUE', True],
            ['FALSE', 'FALSE', False],
            ['NULL', 'FALSE', None],
            ['FALSE', 'NULL', None],
            ['TRUE', 'NULL', None],
            ['NULL', 'TRUE', None],
            ['NULL', 'NULL', None]
        ]
        for i in range(0, len(scenarios), 3):
            lhs = scenarios[i][0]
            rhs = scenarios[i][1]
            expected = scenarios[i][2]
            actual_result = graph.query(f"RETURN {lhs} AND {rhs}").result_set[0][0]
            self.env.assertEquals(actual_result, expected)
    
    def test47_OR(self):
        scenarios = [
            ['TRUE', 'FALSE', True],
            ['FALSE', 'TRUE', True],
            ['TRUE', 'TRUE', True],
            ['FALSE', 'FALSE', False],
            ['NULL', 'FALSE', None],
            ['FALSE', 'NULL', None],
            ['TRUE', 'NULL', True],
            ['NULL', 'TRUE', True],
            ['NULL', 'NULL', None]
        ]
        for i in range(len(scenarios)):
            lhs = scenarios[i][0]
            rhs = scenarios[i][1]
            expected = scenarios[i][2]
            actual_result = graph.query(f"RETURN {lhs} OR {rhs}").result_set[0][0]
            self.env.assertEquals(actual_result, expected)
    
    def test48_XOR(self):
        scenarios = [
            ['TRUE', 'FALSE', True],
            ['FALSE', 'TRUE', True],
            ['TRUE', 'TRUE', False],
            ['FALSE', 'FALSE', False],
            ['NULL', 'FALSE', None],
            ['FALSE', 'NULL', None],
            ['TRUE', 'NULL', None],
            ['NULL', 'TRUE', None],
            ['NULL', 'NULL', None]
        ]
        for i in range(len(scenarios)):
            lhs = scenarios[i][0]
            rhs = scenarios[i][1]
            expected = scenarios[i][2]
            actual_result = graph.query(f"RETURN {lhs} XOR {rhs}").result_set[0][0]
            self.env.assertEquals(actual_result, expected)

    def test49_NOT(self):
        scenarios = [
            ['TRUE', False],
            ['FALSE', True],
            ['NULL', None]
        ]
        for i in range(len(scenarios)):
            b = scenarios[i][0]
            expected = scenarios[i][1]
            actual_result = graph.query(f"RETURN NOT {b}").result_set[0][0]
            self.env.assertEquals(actual_result, expected)
    
    def test50_LT(self):
        scenarios = [
            ['1','1', False],
            ['1', '2', True],
            ['2', '1', False],
            ['2', 'NULL', None],
            ['2', '2', False],
            ['1', 'NULL', None],
            ['NULL', '2', None],
            ['NULL', '1', None],
            ['NULL', 'NULL', None]
        ]
        for i in range(len(scenarios)):
            lhs = scenarios[i][0]
            rhs = scenarios[i][1]
            expected = scenarios[i][2]
            actual_result = graph.query(f"RETURN {lhs} < {rhs}").result_set[0][0]
            self.env.assertEquals(actual_result, expected)
    
    def test51_LE(self):
        scenarios = [
            ['1','1', True],
            ['1', '2', True],
            ['2', '1', False],
            ['2', 'NULL', None],
            ['2', '2', True],
            ['1', 'NULL', None],
            ['NULL', '2', None],
            ['NULL', '1', None],
            ['NULL', 'NULL', None]
        ]
        for i in range(len(scenarios)):
            lhs = scenarios[i][0]
            rhs = scenarios[i][1]
            expected = scenarios[i][2]
            actual_result = graph.query(f"RETURN {lhs} <= {rhs}").result_set[0][0]
            self.env.assertEquals(actual_result, expected)

    def test52_EQ(self):
        scenarios = [
            ['1','1', True],
            ['1', '2', False],
            ['2', '1', False],
            ['2', 'NULL', None],
            ['2', '2', True],
            ['1', 'NULL', None],
            ['NULL', '2', None],
            ['NULL', '1', None],
            ['NULL', 'NULL', None]
        ]
        for i in range(len(scenarios)):
            lhs = scenarios[i][0]
            rhs = scenarios[i][1]
            expected = scenarios[i][2]
            actual_result = graph.query(f"RETURN {lhs} = {rhs}").result_set[0][0]
            self.env.assertEquals(actual_result, expected)
    
    def test53_NE(self):
        scenarios = [
            ['1','1', False],
            ['1', '2', True],
            ['2', '1', True],
            ['2', 'NULL', None],
            ['2', '2', False],
            ['1', 'NULL', None],
            ['NULL', '2', None],
            ['NULL', '1', None],
            ['NULL', 'NULL', None]
        ]
        for i in range(len(scenarios)):
            lhs = scenarios[i][0]
            rhs = scenarios[i][1]
            expected = scenarios[i][2]
            actual_result = graph.query(f"RETURN {lhs} <> {rhs}").result_set[0][0]
            self.env.assertEquals(actual_result, expected)
    
    def test54_List(self):
        arr = [1, 2.3, '4', True, False, None]
        query = "RETURN [1,2.3,'4',TRUE,FALSE, NULL]"
        actual_result = graph.query(query).result_set[0][0]
        if not type(actual_result) is list:
            assert(False)                   # Fail if the record returned is not a list.
        for i in range(len(arr)):
            self.env.assertEquals(actual_result[i], arr[i])

    def test55_ListSlice(self):
        arr = [0,1,2,3,4,5,6,7,8,9,10]
        query_to_expected_result = {
            "RETURN [0,1,2,3,4,5,6,7,8,9,10][3]": [[arr[3]]],
            "RETURN [0,1,2,3,4,5,6,7,8,9,10][-3]": [[arr[-3]]],
            "RETURN [0,1,2,3,4,5,6,7,8,9,10][0..3]": [[arr[0:3]]],
            "RETURN [0,1,2,3,4,5,6,7,8,9,10][0..-5]": [[arr[0:-5]]],
            "RETURN [0,1,2,3,4,5,6,7,8,9,10][-5..]": [[arr[-5:]]],
            "RETURN [0,1,2,3,4,5,6,7,8,9,10][..4]": [[arr[:4]]]
        }
        for query, expected_result in query_to_expected_result.items():
            self.get_res_and_assertEquals(query, expected_result)
    
    def test56_Range(self):
        query_to_expected_result = {
            "RETURN range(0,10)": [[[i for i in range(11)]]],
            "RETURN range(2,18,3)": [[[i for i in range(2, 18, 3)]]]
        }
        for query, expected_result in query_to_expected_result.items():
            self.get_res_and_assertEquals(query, expected_result)
    
    def test57_IN(self):
        query_to_expected_result = {
            "RETURN 3 IN [1,2,3]": [[True]],
            "RETURN 4 IN [1,2,3]": [[False]],
            "RETURN [1,2] IN [1,2,3]": [[False]],
            "RETURN [1,2] IN [[1,2],3]": [[True]]
        }
        for query, expected_result in query_to_expected_result.items():
            self.get_res_and_assertEquals(query, expected_result)

    def test58_ISNULL(self):
        arr = ["NULL", "1", "1.2", "TRUE", "FALSE", "'string'", "[1,2,3]"]
        for ind, s in enumerate(arr):
            query1 = f'RETURN {s} IS NOT NULL'
            expected1 = [[True]] if ind!=0 else [[False]]
            self.get_res_and_assertEquals(query1, expected1)
            query2 = f'RETURN {s} IS NULL'
            expected2 = [[False]] if ind!=0 else [[True]]
            self.get_res_and_assertEquals(query2, expected2)
    
    def test59_Coalesce(self):
        query_to_expected_result = {
            "RETURN coalesce(1)": [[1]],
            "RETURN coalesce(NULL, 1)": [[1]],
            "RETURN coalesce(NULL, NULL, 500, NULL)": [[500]]
        }
        for query, expected_result in query_to_expected_result.items():
            self.get_res_and_assertEquals(query, expected_result)
    
    def test60_Replace(self):
        query_to_expected_result = {
            "RETURN replace('abcabc', 'a', '00')": [["00bc00bc"]],
            "RETURN replace('abcabc', 'bc', '0')": [["a0a0"]],
            "RETURN replace('abcabc', 'abc', '')": [[""]],
            "RETURN replace('abcabc', 'ab', '')": [["cc"]],
            "RETURN replace('abcabc', '', '0')": [["0a0b0c0a0b0c0"]]
        }
        for query, expected_result in query_to_expected_result.items():
            self.get_res_and_assertEquals(query, expected_result)

    def test61_RandomUUID(self):
        query = "RETURN randomUUID()"
        actual_result = graph.query(query).result_set[0][0]
        self.env.assertEquals(actual_result[8], '-')
        self.env.assertEquals(actual_result[13], '-')
        self.env.assertEquals(actual_result[14], '4')
        self.env.assertEquals(actual_result[18], '-')
        if not actual_result[19] in ['8', '9', 'a', 'b']:
            assert(False)
        self.env.assertEquals(actual_result[23], '-')

    def test62_division_inputs(self):
        # Validate integer dividend division by 0
        # redis-cli output example:
        # 127.0.0.1:6379> GRAPH.QUERY g "RETURN 3 / 0"
        # (error) Division by zero
        query = "RETURN 3 / 0"
        try:
            actual_result = graph.query(query)
        except redis.ResponseError as e:
            self.env.assertContains("Division by zero", str(e))
        
        # Validate floating-point dividend division by 0
        # redis-cli output example:
        # 127.0.0.1:6379> GRAPH.QUERY g "RETURN 1.0 / 0"
        # 1) 1) "1.0 / 0"
        # 2) 1) 1) "inf"
        query = "RETURN 1.0 / 0"
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0],float('inf'))

        # Validate negative floating-point dividend division by 0
        # redis-cli output example:
        # 127.0.0.1:6379> GRAPH.QUERY g "RETURN -1.0 / 0"
        # 1) 1) "-1.0 / 0"
        # 2) 1) 1) "-inf"
        query = "RETURN -1.0 / 0"
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0],float('-inf'))

        # Validate integer dividend division by 0.0 
        # redis-cli output example:
        # 127.0.0.1:6379> GRAPH.QUERY g "RETURN 1 / 0.0"
        # 1) 1) "1 / 0.0"
        # 2) 1) 1) "inf"
        query = "RETURN 1 / 0.0"
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0],float('inf'))

        # Validate negative integer dividend division by 0.0 
        # redis-cli output example:
        # 127.0.0.1:6379> GRAPH.QUERY g "RETURN -1 / 0.0"
        # 1) 1) "-1 / 0.0"
        # 2) 1) 1) "-inf"
        query = "RETURN -1 / 0.0"
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0],float('-inf'))

        # Validate floating-point dividend division by 0.0 
        # redis-cli output example:
        # 127.0.0.1:6379> GRAPH.QUERY g "RETURN 1.0 / 0.0"
        # 1) 1) "1.0 / 0.0"
        # 2) 1) 1) "inf"
        query = "RETURN 1.0 / 0.0"
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0],float('inf'))

        # Validate negative floating-point dividend division by 0.0 
        # redis-cli output example:
        # 127.0.0.1:6379> GRAPH.QUERY g "RETURN -1.0 / 0.0"
        # 1) 1) "-1.0 / 0.0"
        # 2) 1) 1) "-inf"
        query = "RETURN -1.0 / 0.0"
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0],float('-inf'))

        # Validate floating-point 0.0 divided by floating-point 0.0 
        # redis-cli output example:
        # 127.0.0.1:6379> GRAPH.QUERY g "RETURN 0.0 / 0.0"
        # 1) 1) "0.0 / 0.0"
        # 2) 1) 1) "-nan"
        query = "RETURN 0.0 / 0.0"
        actual_result = graph.query(query)
        self.env.assertTrue(math.isnan(actual_result.result_set[0][0]))

        # Validate integer 0 divided by floating 0.0 
        # redis-cli output example:
        # 127.0.0.1:6379> GRAPH.QUERY g "RETURN 0 / 0.0"
        # 1) 1) "0 / 0.0"
        # 2) 1) 1) "-nan"
        query = "RETURN 0 / 0.0"
        actual_result = graph.query(query)
        self.env.assertTrue(math.isnan(actual_result.result_set[0][0]))

        # Validate floating-point 0.0 divided by integer 0 
        # redis-cli output example:
        # 127.0.0.1:6379> GRAPH.QUERY g "RETURN 0.0 / 0"
        # 1) 1) "0.0 / 0"
        # 2) 1) 1) "-nan"
        query = "RETURN 0.0 / 0"
        actual_result = graph.query(query)
        self.env.assertTrue(math.isnan(actual_result.result_set[0][0]))

        # Validate integer 0 divided by 0 
        # redis-cli output example:
        # 127.0.0.1:6379> GRAPH.QUERY g "RETURN 0 / 0"
        # (error) Division by zero
        query = "RETURN 0 / 0"
        try:
            actual_result = graph.query(query)
        except redis.ResponseError as e:
            self.env.assertContains("Division by zero", str(e))

    def test63_in_out_degree(self):
        # clear graph
        self.env.flush()

        # given the graph (a)
        # in/out degree of 'a' is 0
        graph.query("CREATE (a:A)")
        result_set = graph.query("MATCH (a:A) RETURN inDegree(a), outDegree(a)").result_set
        in_degree = result_set[0][0]
        out_degree = result_set[0][1]
        self.env.assertEqual(in_degree, 0)
        self.env.assertEqual(out_degree, 0)

        # given the graph: (a)-[:E]->(b)
        # in degree of 'a' is 0
        # out degree of 'a' is 1
        # in degree of 'b' is 1
        # out degree of 'b' is 0
        graph.query("MATCH (a:A) CREATE (a)-[:E]->(b:B)")
        result_set = graph.query("MATCH (a:A), (b:B) RETURN inDegree(a), outDegree(a), inDegree(b), outDegree(b)").result_set
        a_in_degree = result_set[0][0]
        a_out_degree = result_set[0][1]
        b_in_degree = result_set[0][2]
        b_out_degree = result_set[0][3]
        self.env.assertEqual(a_in_degree, 0)
        self.env.assertEqual(a_out_degree, 1)
        self.env.assertEqual(b_in_degree, 1)
        self.env.assertEqual(b_out_degree, 0)

        # given the graph (a)-[:E]->(b), (a)-[:E]->(b)
        # in degree of 'a' is 0
        # out degree of 'a' is 2
        # in degree of 'b' is 2
        # out degree of 'b' is 0
        graph.query("MATCH (a:A), (b:B) CREATE (a)-[:E]->(b)")
        result_set = graph.query("MATCH (a:A), (b:B) RETURN inDegree(a), outDegree(a), inDegree(b), outDegree(b)").result_set
        a_in_degree = result_set[0][0]
        a_out_degree = result_set[0][1]
        b_in_degree = result_set[0][2]
        b_out_degree = result_set[0][3]
        self.env.assertEqual(a_in_degree, 0)
        self.env.assertEqual(a_out_degree, 2)
        self.env.assertEqual(b_in_degree, 2)
        self.env.assertEqual(b_out_degree, 0)

        # given the graph (a)-[:E]->(b), (a)-[:E]->(b) (a)-[:E0]->(b), (a)-[:E1]->(b)
        # in degree of 'a' is 0
        # out degree of 'a' is 4
        # in degree of 'b' is 4
        # out degree of 'b' is 0
        graph.query("MATCH (a:A), (b:B) CREATE (a)-[:E0]->(b), (a)-[:E1]->(b)")
        result_set = graph.query("MATCH (a:A), (b:B) RETURN inDegree(a), outDegree(a), inDegree(b), outDegree(b)").result_set
        a_in_degree = result_set[0][0]
        a_out_degree = result_set[0][1]
        b_in_degree = result_set[0][2]
        b_out_degree = result_set[0][3]
        self.env.assertEqual(a_in_degree, 0)
        self.env.assertEqual(a_out_degree, 4)
        self.env.assertEqual(b_in_degree, 4)
        self.env.assertEqual(b_out_degree, 0)

        # given the graph (a)-[:E]->(b), (a)-[:E]->(b) (a)-[:E0]->(b), (a)-[:E1]->(b)
        # in degree of 'a' for relation 'E' is 0
        # out degree of 'a' for relation 'E' is 2
        # in degree of 'b' for relation 'E' is 2
        # out degree of 'b' for relation 'E' is 0
        result_set = graph.query("MATCH (a:A), (b:B) RETURN inDegree(a, 'E'), outDegree(a, 'E'), inDegree(b, 'E'), outDegree(b, 'E')").result_set
        a_in_degree = result_set[0][0]
        a_out_degree = result_set[0][1]
        b_in_degree = result_set[0][2]
        b_out_degree = result_set[0][3]
        self.env.assertEqual(a_in_degree, 0)
        self.env.assertEqual(a_out_degree, 2)
        self.env.assertEqual(b_in_degree, 2)
        self.env.assertEqual(b_out_degree, 0)

        # given the graph (a)-[:E]->(b), (a)-[:E]->(b) (a)-[:E0]->(b), (a)-[:E1]->(b)
        # in degree of 'a' for relationships 'E0' and 'E1' is 0
        # out degree of 'a' for relationships 'E0' and 'E1' is 2
        # in degree of 'b' for relationships 'E0' and 'E1' is 2
        # out degree of 'b' for relationships 'E0' and 'E1' is 0
        result_set = graph.query("MATCH (a:A), (b:B) RETURN inDegree(a, 'E0', 'E1'), outDegree(a, 'E0', 'E1'), inDegree(b, 'E0', 'E1'), outDegree(b, 'E0', 'E1')").result_set
        a_in_degree = result_set[0][0]
        a_out_degree = result_set[0][1]
        b_in_degree = result_set[0][2]
        b_out_degree = result_set[0][3]
        self.env.assertEqual(a_in_degree, 0)
        self.env.assertEqual(a_out_degree, 2)
        self.env.assertEqual(b_in_degree, 2)
        self.env.assertEqual(b_out_degree, 0)

        # given the graph (a)-[:E]->(b), (a)-[:E]->(b) (a)-[:E0]->(b), (a)-[:E1]->(b)
        # in degree of 'a' for relationships 'E', 'E0' and 'E1' is 0
        # out degree of 'a' for relationships 'E', 'E0' and 'E1' is 4
        # in degree of 'b' for relationships 'E', 'E0' and 'E1' is 4
        # out degree of 'b' for relationships 'E', 'E0' and 'E1' is 0
        result_set = graph.query("MATCH (a:A), (b:B) RETURN inDegree(a, 'E', 'E0', 'E1'), outDegree(a, 'E', 'E0', 'E1'), inDegree(b, 'E', 'E0', 'E1'), outDegree(b, 'E', 'E0', 'E1')").result_set
        a_in_degree = result_set[0][0]
        a_out_degree = result_set[0][1]
        b_in_degree = result_set[0][2]
        b_out_degree = result_set[0][3]
        self.env.assertEqual(a_in_degree, 0)
        self.env.assertEqual(a_out_degree, 4)
        self.env.assertEqual(b_in_degree, 4)
        self.env.assertEqual(b_out_degree, 0)

        # in/out degree a none existing relationship type is 0
        result_set = graph.query("MATCH (a:A) RETURN inDegree(a, 'none_existing'), outDegree(a, 'none_existing')").result_set
        in_degree = result_set[0][0]
        out_degree = result_set[0][1]
        self.env.assertEqual(in_degree, 0)
        self.env.assertEqual(out_degree, 0)

        # clear graph
        self.env.flush()

        # given the graph (a)-[:E]->(a)
        # in/out degree of 'a' is 1
        graph.query("CREATE (a)-[:E]->(a)")
        result_set = graph.query("MATCH (a) RETURN inDegree(a), outDegree(a)").result_set
        in_degree = result_set[0][0]
        out_degree = result_set[0][1]
        self.env.assertEqual(in_degree, 1)
        self.env.assertEqual(out_degree, 1)

