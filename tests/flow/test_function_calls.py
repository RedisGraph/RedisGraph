from common import *
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

    def test28_sqrt(self):
        query = """RETURN sqrt(0)"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], 0)

        query = """RETURN sqrt(9801)"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], 99)

        query = """RETURN sqrt(-1)"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], None)

        query = """RETURN sqrt(-9)"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], None)

        query = """RETURN sqrt(-0.0000000001)"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], None)

        query = """RETURN sqrt(null)"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], None)

        query = """RETURN sqrt(2540.95581553)"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], 50.4078943770715)
    
    def test29_toBoolean(self):
        # all other toBoolean cases (boolean, strings, null, errors) are covered in TCK
        # integers
        query = """RETURN toBoolean(0)"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], False)
        query = """RETURN toBoolean(1)"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], True)
        query = """RETURN toBoolean(-1)"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], True)

    def test29_toBooleanOrNull(self):
        # boolean
        query = """RETURN toBooleanOrNull(true)"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], True)
        query = """RETURN toBooleanOrNull(false)"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], False)

        # strings
        query = """RETURN toBooleanOrNull('TruE')"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], True)
        query = """RETURN toBooleanOrNull('FaLsE')"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], False)
        query = """RETURN toBooleanOrNull('not a boolean')"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], None)

        # integers
        query = """RETURN toBooleanOrNull(0)"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], False)
        query = """RETURN toBooleanOrNull(1)"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], True)
        query = """RETURN toBooleanOrNull(-1)"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], True)

        # null
        query = """RETURN toBooleanOrNull(null)"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], None)

        # float
        query = """RETURN toBooleanOrNull(0.1)"""
        graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], None)

        # list
        query = """RETURN toBooleanOrNull([true])"""
        graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], None)

        # node
        query = """CREATE (n) RETURN toBooleanOrNull(n)"""
        graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], None)

        # edge
        query = """CREATE ()-[r:R]->() RETURN toBooleanOrNull(r)"""
        graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], None)

    def test30_toFloatOrNull(self):
        # floats
        query = """RETURN toFloatOrNull(1.2)"""
        actual_result = graph.query(query)
        self.env.assertAlmostEqual(actual_result.result_set[0][0], 1.2, 0.0001)

        # strings
        query = """RETURN toFloatOrNull('1.23')"""
        actual_result = graph.query(query)
        self.env.assertAlmostEqual(actual_result.result_set[0][0], 1.23, 0.0001)
        query = """RETURN toFloatOrNull('1.2.3')"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], None)
        query = """RETURN toFloatOrNull('not a boolean')"""

        # integers
        query = """RETURN toFloatOrNull(0.1)"""
        actual_result = graph.query(query)
        self.env.assertAlmostEqual(actual_result.result_set[0][0], 0.1, 0.0001)
        query = """RETURN toFloatOrNull(1)"""
        actual_result = graph.query(query)
        self.env.assertAlmostEqual(actual_result.result_set[0][0], 1.0, 0.0001)
        query = """RETURN toFloatOrNull(-1)"""
        actual_result = graph.query(query)
        self.env.assertAlmostEqual(actual_result.result_set[0][0], -1.0, 0.0001)

        # null
        query = """RETURN toFloatOrNull(null)"""
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], None)

        # boolean
        query = """RETURN toFloatOrNull(true)"""
        graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], None)
        query = """RETURN toFloatOrNull(false)"""
        graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], None)

        # list
        query = """RETURN toFloatOrNull([1.0])"""
        graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], None)

        # node
        query = """CREATE (n) RETURN toFloatOrNull(n)"""
        graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], None)

        # edge
        query = """CREATE ()-[r:R]->() RETURN toFloatOrNull(r)"""
        graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], None)
