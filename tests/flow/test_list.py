from common import *

redis_graph = None
GRAPH_ID = "G"


# tests the GRAPH.LIST command
class testGraphList(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)

    def create_graph(self, graph_name, con):
        con.execute_command("GRAPH.QUERY", graph_name, "RETURN 1")

    def test_graph_list(self):
        # no graphs, expecting an empty array
        con = self.env.getConnection()
        graphs = con.execute_command("GRAPH.LIST")
        self.env.assertEquals(graphs, [])

        # create graph key "G"
        self.create_graph("G", con)
        graphs = con.execute_command("GRAPH.LIST")
        self.env.assertEquals(graphs, ["G"])

        # create a second graph key "X"
        self.create_graph("X", con)
        graphs = con.execute_command("GRAPH.LIST")
        graphs.sort()
        self.env.assertEquals(graphs, ["G", "X"])

        # create a string key "str", graph list shouldn't be effected
        con.set("str", "some string")
        graphs = con.execute_command("GRAPH.LIST")
        graphs.sort()
        self.env.assertEquals(graphs, ["G", "X"])

        # delete graph key "G"
        con.delete("G")
        graphs = con.execute_command("GRAPH.LIST")
        self.env.assertEquals(graphs, ["X"])

        # rename graph key X to Z
        con.rename("X", "Z")
        graphs = con.execute_command("GRAPH.LIST")
        self.env.assertEquals(graphs, ["Z"])

        # delete graph key "Z", no graph keys in the keyspace
        con.execute_command("GRAPH.DELETE", "Z")
        graphs = con.execute_command("GRAPH.LIST")
        self.env.assertEquals(graphs, [])

# tests the list datatype
class testList(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(redis_con, GRAPH_ID)

    def expect_error(self, query, expected_err_msg):
        try:
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertIn(expected_err_msg, str(e))

    def test01_collect(self):
        for i in range(10):
            redis_graph.add_node(Node())
        redis_graph.commit()

        query = """MATCH (n) RETURN collect(n)"""
        result = redis_graph.query(query)
        result_set = result.result_set
        self.env.assertEquals(len(result_set), 1)
        self.env.assertTrue(all(isinstance(n, Node) for n in result_set[0][0]))

    def test02_unwind(self):
        query = """CREATE ()"""
        redis_graph.query(query)
        query = """unwind(range(0,10)) as x return x"""
        result_set = redis_graph.query(query).result_set
        expected_result = [[0], [1], [2], [3],
                           [4], [5], [6], [7], [8], [9], [10]]
        self.env.assertEquals(result_set, expected_result)

    # List functions should handle null inputs appropriately.
    def test03_null_list_function_inputs(self):
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

        # NULL list argument to LAST.
        query = """WITH NULL as list RETURN last(list)"""
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

    def test04_head_function(self):
        # Test empty list input
        query = """RETURN head([])"""
        actual_result = redis_graph.query(query)
        expected_result = [[None]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test integer list input
        query = """WITH [1, 2, 3] as list RETURN head(list)"""
        actual_result = redis_graph.query(query)
        expected_result = [[1]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test string list input
        query = """WITH ['a', 'b', 'c'] as list RETURN head(list)"""
        actual_result = redis_graph.query(query)
        expected_result = [['a']]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test NULL value in list input
        query = """WITH [NULL, 'b', NULL] as list RETURN head(list)"""
        actual_result = redis_graph.query(query)
        expected_result = [[None]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test wrong type inputs
        queries_with_errors = {
            "RETURN head(1)" : "Type mismatch on function 'head' argument 1: expected List or Null but was Integer",
            "RETURN head('a')" : "Type mismatch on function 'head' argument 1: expected List or Null but was String",
            "RETURN head(true)" : "Type mismatch on function 'head' argument 1: expected List or Null but was Boolean",
        }
        for query, error in queries_with_errors.items():
            self.expect_error(query, error)

    def test05_last_function(self):
        # Test empty list input
        query = """RETURN last([])"""
        actual_result = redis_graph.query(query)
        expected_result = [[None]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test integer list input
        query = """WITH [1, 2, 3] as list RETURN last(list)"""
        actual_result = redis_graph.query(query)
        expected_result = [[3]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test string list input
        query = """WITH ['a', 'b', 'c'] as list RETURN last(list)"""
        actual_result = redis_graph.query(query)
        expected_result = [['c']]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test NULL value in list input
        query = """WITH [NULL, 'b', NULL] as list RETURN last(list)"""
        actual_result = redis_graph.query(query)
        expected_result = [[None]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test wrong type inputs
        queries_with_errors = {
            "RETURN last(1)" : "Type mismatch on function 'last' argument 1: expected List or Null but was Integer",
            "RETURN last('a')" : "Type mismatch on function 'last' argument 1: expected List or Null but was String",
            "RETURN last(true)" : "Type mismatch on function 'last' argument 1: expected List or Null but was Boolean",
        }
        for query, error in queries_with_errors.items():
            self.expect_error(query, error)

    def test06_toBooleanList(self):
        # NULL input should return NULL
        expected_result = [None]
        query = """RETURN toBooleanList(null)"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0], expected_result)

        # List of NULL values input should return list of NULL values
        query = """RETURN toBooleanList([null, null])"""
        expected_result = [[None, None]]
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0], expected_result)

        # Test list with mixed type values
        query = """RETURN toBooleanList(['abc', true, 'false', null, ['a','b']])"""
        expected_result = [[None, True, False, None, None]]
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0], expected_result)

        # Tests with empty array result
        query = """MATCH (n:X) RETURN toBooleanList(n)"""
        expected_result = []
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """MATCH (n:X) RETURN toBooleanList([n])"""
        expected_result = []
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """MATCH ()-[e]->() RETURN toBooleanList(e)"""
        expected_result = []
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """MATCH ()-[e]->() RETURN toBooleanList([e])"""
        expected_result = []
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """MATCH p=()-[]->() RETURN toBooleanList(p)"""
        expected_result = []
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """MATCH p=()-[]->() RETURN toBooleanList([p])"""
        expected_result = []
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test of type mismatch input
        # A list of queries and errors which are expected to occur with the specified query.
        queries_with_errors = {
            "RETURN toBooleanList(true)" : "Type mismatch on function 'toBooleanList' argument 1: expected List or Null but was Boolean",
            "RETURN toBooleanList(7.05)" : "Type mismatch on function 'toBooleanList' argument 1: expected List or Null but was Float",
            "RETURN toBooleanList(7)"    : "Type mismatch on function 'toBooleanList' argument 1: expected List or Null but was Integer",
            "RETURN toBooleanList('abc')" : "Type mismatch on function 'toBooleanList' argument 1: expected List or Null but was String",
            "CREATE (a:X) RETURN toBooleanList(a)" : "Type mismatch on function 'toBooleanList' argument 1: expected List or Null but was Node",
            "CREATE (a:X)-[r:R]->(b:Y) RETURN toBooleanList(r)" : "Type mismatch on function 'toBooleanList' argument 1: expected List or Null but was Edge",
        }
        for query, error in queries_with_errors.items():
            self.expect_error(query, error)
        
        try:
            query = """CREATE (a:X), (b:Y)"""
            redis_graph.query(query)
            query = """MATCH (a:X), (b:Y) CREATE (a)-[r:R]->(b)"""
            redis_graph.query(query)
            query = """MATCH p=()-[]->() RETURN toBooleanList(p)"""
            actual_result = redis_graph.query(query)
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch on function 'toBooleanList' argument 1: expected List or Null but was Path", str(e))
            query = """MATCH (a:X),(b:Y) DELETE a, b"""
            redis_graph.query(query)

        # List of node input should return list of NULL
        query = """CREATE (a:X) RETURN toBooleanList([a])"""
        expected_result = [[None]]
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0], expected_result)
        query = """MATCH (a:X) DELETE a"""
        redis_graph.query(query)

        # List of edge input should return list of NULL
        query = """CREATE (a:X)-[r:R]->(b:Y) RETURN toBooleanList([r])"""
        expected_result = [[None]]
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0], expected_result)
        query = """MATCH (a:X),(b:Y) DELETE a, b"""
        redis_graph.query(query)

        # List of path input should return list of NULL
        query = """CREATE (a:X), (b:Y)"""
        redis_graph.query(query)
        query = """MATCH (a:X), (b:Y) create (a)-[e:R]->(b)"""
        redis_graph.query(query)
        query = """MATCH p=()-[]->() RETURN toBooleanList([p])"""
        expected_result = [[None]]
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0], expected_result)
        query = """MATCH (a:X),(b:Y) DELETE a, b"""
        redis_graph.query(query)

        # Test without input argument
        try:
            query = """RETURN toBooleanList()"""
            redis_graph.query(query)
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Received 0 arguments to function 'toBooleanList', expected at least 1", str(e)) 

    def test07_toFloatList(self):
        # NULL input should return NULL
        expected_result = [None]
        query = """RETURN toFloatList(null)"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0], expected_result)

        # List of NULL values input should return list of NULL values
        query = """RETURN toFloatList([null, null])"""
        expected_result = [[None, None]]
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0], expected_result)

        # Test list with mixed type values
        query = """RETURN toFloatList(['abc', 1.5, 7.0578, null, ['a','b']]) """
        expected_result = [[None, 1.5, 7.0578, None, None]]
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0], expected_result)

        query = """RETURN toFloatList(['7.0578']) """
        expected_result = 7.0578
        actual_result = redis_graph.query(query)
        self.env.assertAlmostEqual(float(actual_result.result_set[0][0][0]), expected_result, 1e-5)

        # Tests with empty array result
        query = """MATCH (n:X) RETURN toFloatList(n)"""
        expected_result = []
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """MATCH (n:X) RETURN toFloatList([n])"""
        expected_result = []
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """MATCH ()-[e]->() RETURN toFloatList(e)"""
        expected_result = []
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """MATCH ()-[e]->() RETURN toFloatList([e])"""
        expected_result = []
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """MATCH p=()-[]->() RETURN toFloatList(p)"""
        expected_result = []
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """MATCH p=()-[]->() RETURN toFloatList([p])"""
        expected_result = []
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test of type mismatch input
        # A list of queries and errors which are expected to occur with the specified query.
        queries_with_errors = {
            "RETURN toFloatList(true)" : "Type mismatch on function 'toFloatList' argument 1: expected List or Null but was Boolean",
            "RETURN toFloatList(7.05)" : "Type mismatch on function 'toFloatList' argument 1: expected List or Null but was Float",
            "RETURN toFloatList(7)"    : "Type mismatch on function 'toFloatList' argument 1: expected List or Null but was Integer",
            "RETURN toFloatList('abc')" : "Type mismatch on function 'toFloatList' argument 1: expected List or Null but was String",
            "CREATE (a:X) RETURN toFloatList(a)" : "Type mismatch on function 'toFloatList' argument 1: expected List or Null but was Node",
            "CREATE (a:X)-[r:R]->(b:Y) RETURN toFloatList(r)" : "Type mismatch on function 'toFloatList' argument 1: expected List or Null but was Edge",
        }
        for query, error in queries_with_errors.items():
            self.expect_error(query, error)

        try:
            query = """CREATE (a:X), (b:Y)"""
            redis_graph.query(query)
            query = """MATCH (a:X), (b:Y) CREATE (a)-[r:R]->(b)"""
            redis_graph.query(query)
            query = """MATCH p=()-[]->() RETURN toFloatList(p)"""
            actual_result = redis_graph.query(query)
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch on function 'toFloatList' argument 1: expected List or Null but was Path", str(e))
            query = """MATCH (a:X),(b:Y) DELETE a, b"""
            redis_graph.query(query)

        # List of node input should return list of NULL
        query = """CREATE (a:X) RETURN toFloatList([a])"""
        expected_result = [[None]]
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0], expected_result)
        query = """MATCH (a:X) DELETE a"""
        redis_graph.query(query)

        # List of edge input should return list of NULL
        query = """CREATE (a:X)-[r:R]->(b:Y) RETURN toFloatList([r])"""
        expected_result = [[None]]
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0], expected_result)
        query = """MATCH (a:X),(b:Y) DELETE a, b"""
        redis_graph.query(query)

        # List of path input should return list of NULL
        query = """CREATE (a:X), (b:Y)"""
        redis_graph.query(query)
        query = """MATCH (a:X), (b:Y) create (a)-[e:R]->(b)"""
        redis_graph.query(query)
        query = """MATCH p=()-[]->() RETURN toFloatList([p])"""
        expected_result = [[None]]
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0], expected_result)
        query = """MATCH (a:X), (b:Y) DELETE a, b"""
        redis_graph.query(query)

        # Test without input argument
        try:
            query = """RETURN toFloatList()"""
            redis_graph.query(query)
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Received 0 arguments to function 'toFloatList', expected at least 1", str(e))

    def test08_toIntegerList(self):
        # NULL input should return NULL
        expected_result = [None]
        query = """RETURN toIntegerList(null)"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0], expected_result)

        # List of NULL values input should return list of NULL values
        query = """RETURN toIntegerList([null, null])"""
        expected_result = [[None, None]]
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0], expected_result)

        # Test list with mixed type values
        query = """RETURN toIntegerList(['abc', 7, '5', null, ['a','b']]) """
        expected_result = [[None, 7, 5, None, None]]
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0], expected_result)

        # Tests with empty array result
        query = """MATCH (n:X) RETURN toIntegerList(n)"""
        expected_result = []
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """MATCH (n:X) RETURN toIntegerList([n])"""
        expected_result = []
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """MATCH ()-[e]->() RETURN toIntegerList(e)"""
        expected_result = []
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """MATCH ()-[e]->() RETURN toIntegerList([e])"""
        expected_result = []
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """MATCH p=()-[]->() RETURN toIntegerList(p)"""
        expected_result = []
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """MATCH p=()-[]->() RETURN toIntegerList([p])"""
        expected_result = []
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test of type mismatch input
        # A list of queries and errors which are expected to occur with the specified query.
        queries_with_errors = {
            "RETURN toIntegerList(true)" : "Type mismatch on function 'toIntegerList' argument 1: expected List or Null but was Boolean",
            "RETURN toIntegerList(7.05)" : "Type mismatch on function 'toIntegerList' argument 1: expected List or Null but was Float",
            "RETURN toIntegerList(7)"    : "Type mismatch on function 'toIntegerList' argument 1: expected List or Null but was Integer",
            "RETURN toIntegerList('abc')" : "Type mismatch on function 'toIntegerList' argument 1: expected List or Null but was String",
            "CREATE (a:X) RETURN toIntegerList(a)" : "Type mismatch on function 'toIntegerList' argument 1: expected List or Null but was Node",
            "CREATE (a:X)-[r:R]->(b:Y) RETURN toIntegerList(r)" : "Type mismatch on function 'toIntegerList' argument 1: expected List or Null but was Edge",
        }
        for query, error in queries_with_errors.items():
            self.expect_error(query, error)

        try:
            query = """CREATE (a:X), (b:Y)"""
            redis_graph.query(query)
            query = """MATCH (a:X), (b:Y) CREATE (a)-[r:R]->(b)"""
            redis_graph.query(query)
            query = """MATCH p=()-[]->() RETURN toIntegerList(p)"""
            actual_result = redis_graph.query(query)
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch on function 'toIntegerList' argument 1: expected List or Null but was Path", str(e))
            query = """MATCH (a:X),(b:Y) DELETE a, b"""
            redis_graph.query(query)

        # List of node input should return list of NULL
        query = """CREATE (a:X) RETURN toIntegerList([a])"""
        expected_result = [[None]]
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0], expected_result)
        query = """MATCH (a:X) DELETE a"""
        redis_graph.query(query)

        # List of edge input should return list of NULL
        query = """CREATE (a:X)-[r:R]->(b:Y) RETURN toIntegerList([r])"""
        expected_result = [[None]]
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0], expected_result)
        query = """MATCH (a:X),(b:Y) DELETE a, b"""
        redis_graph.query(query)

        # List of path input should return list of NULL
        query = """CREATE (a:X), (b:Y)"""
        redis_graph.query(query)
        query = """MATCH (a:X), (b:Y) create (a)-[e:R]->(b)"""
        redis_graph.query(query)
        query = """MATCH p=()-[]->() RETURN toIntegerList([p])"""
        expected_result = [[None]]
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0], expected_result)
        query = """MATCH (a:X), (b:Y) DELETE a, b"""
        redis_graph.query(query)

        # Test without input argument
        try:
            query = """RETURN toIntegerList()"""
            redis_graph.query(query)
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Received 0 arguments to function 'toIntegerList', expected at least 1", str(e))

    def test09_toStringList(self):
        # NULL input should return NULL
        expected_result = [None]
        query = """RETURN toStringList(null)"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0], expected_result)

        # List of NULL values input should return list of NULL values
        query = """RETURN toStringList([null, null])"""
        expected_result = [[None, None]]
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0], expected_result) 

        # Test list with string values
        query = """RETURN toStringList(['abc', '5.32', 'true', 'this is a test'])"""
        expected_result = [['abc', '5.32', 'true', 'this is a test']]
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0], expected_result)

        # Test list with float values
        query = """RETURN toStringList([0.32425, 5.32, 7.1])"""
        expected_result = [['0.324250', '5.320000', '7.100000']]
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0], expected_result)

        # Test list with mixed type values
        query = """RETURN toStringList(['abc', 7, '5.32', null, ['a','b']]) """
        expected_result = [['abc', '7', '5.32', None, None]]
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0], expected_result)

        # Tests with empty array result
        query = """MATCH (n:X) RETURN toStringList(n)"""
        expected_result = []
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """MATCH (n:X) RETURN toStringList([n])"""
        expected_result = []
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """MATCH ()-[e]->() RETURN toStringList(e)"""
        expected_result = []
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """MATCH ()-[e]->() RETURN toStringList([e])"""
        expected_result = []
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """MATCH p=()-[]->() RETURN toStringList(p)"""
        expected_result = []
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """MATCH p=()-[]->() RETURN toStringList([p])"""
        expected_result = []
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test of type mismatch input
        # A list of queries and errors which are expected to occur with the specified query.
        queries_with_errors = {
            "RETURN toStringList(true)" : "Type mismatch on function 'toStringList' argument 1: expected List or Null but was Boolean",
            "RETURN toStringList(7.05)" : "Type mismatch on function 'toStringList' argument 1: expected List or Null but was Float",
            "RETURN toStringList(7)"    : "Type mismatch on function 'toStringList' argument 1: expected List or Null but was Integer",
            "RETURN toStringList('abc')" : "Type mismatch on function 'toStringList' argument 1: expected List or Null but was String",
            "CREATE (a:X) RETURN toStringList(a)" : "Type mismatch on function 'toStringList' argument 1: expected List or Null but was Node",
            "CREATE (a:X)-[r:R]->(b:Y) RETURN toStringList(r)" : "Type mismatch on function 'toStringList' argument 1: expected List or Null but was Edge",
        }
        for query, error in queries_with_errors.items():
            self.expect_error(query, error)

        try:
            query = """CREATE (a:X), (b:Y)"""
            redis_graph.query(query)
            query = """MATCH (a:X), (b:Y) CREATE (a)-[r:R]->(b)"""
            redis_graph.query(query)
            query = """MATCH p=()-[]->() RETURN toStringList(p)"""
            actual_result = redis_graph.query(query)
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch on function 'toStringList' argument 1: expected List or Null but was Path", str(e))
            query = """MATCH (a:X),(b:Y) DELETE a, b"""
            redis_graph.query(query)

        # List of node input should return list of NULL
        query = """CREATE (a:X) RETURN toStringList([a])"""
        expected_result = [[None]]
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0], expected_result)
        query = """MATCH (a:X) DELETE a"""
        redis_graph.query(query)

        # List of edge input should return list of NULL
        query = """CREATE (a:X)-[r:R]->(b:Y) RETURN toStringList([r])"""
        expected_result = [[None]]
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0], expected_result)
        query = """MATCH (a:X),(b:Y) DELETE a, b"""
        redis_graph.query(query)

        # List of path input should return list of NULL
        query = """CREATE (a:X), (b:Y)"""
        redis_graph.query(query)
        query = """MATCH (a:X), (b:Y) create (a)-[e:R]->(b)"""
        redis_graph.query(query)
        query = """MATCH p=()-[]->() RETURN toStringList([p])"""
        expected_result = [[None]]
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0], expected_result)
        query = """MATCH (a:X), (b:Y) DELETE a, b"""
        redis_graph.query(query)

        # Test without input argument
        try:
            query = """RETURN toStringList()"""
            redis_graph.query(query)
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Received 0 arguments to function 'toStringList', expected at least 1", str(e))
