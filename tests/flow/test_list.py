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

    def test04_toBooleanList(self):
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
        try:
            redis_graph.query("RETURN toBooleanList(true)")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch: expected List but was Boolean", str(e))

        try:
            redis_graph.query("RETURN toBooleanList(7.05)")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch: expected List but was Float", str(e))

        try:
            redis_graph.query("RETURN toBooleanList(7)")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch: expected List but was Integer", str(e))

        try:
            redis_graph.query("RETURN toBooleanList('abc')")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch: expected List but was String", str(e))

        try:
            query = """CREATE (a:X)"""
            redis_graph.query(query)
            query = """MATCH (n) RETURN toBooleanList(n)"""
            actual_result = redis_graph.query(query)
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch: expected List but was Node", str(e))
            query = """MATCH (a:X) DELETE a"""
            redis_graph.query(query)

        try:
            query = """CREATE (a:X), (b:Y)"""
            redis_graph.query(query)
            query = """MATCH (a:X), (b:Y) create (a)-[r:R]->(b)"""
            redis_graph.query(query)
            query = """MATCH ()-[e]->() RETURN toBooleanList(e)"""
            actual_result = redis_graph.query(query)
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch: expected List but was Edge", str(e))
            query = """MATCH (a:X),(b:Y) DELETE a, b"""
            redis_graph.query(query)

        try:
            query = """CREATE (a:X), (b:Y)"""
            redis_graph.query(query)
            query = """MATCH (a:X), (b:Y) CREATE (a)-[r:R]->(b)"""
            redis_graph.query(query)
            query = """MATCH p=()-[]->() RETURN toBooleanList(p)"""
            actual_result = redis_graph.query(query)
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch: expected List but was Path", str(e))
            query = """MATCH (a:X),(b:Y) DELETE a, b"""
            redis_graph.query(query)

        # List of node input should return list of NULL
        query = """CREATE (a:X)"""
        redis_graph.query(query)
        query = """MATCH (n) RETURN toBooleanList([n])"""
        expected_result = [[None]]
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0], expected_result)
        query = """MATCH (a:X) DELETE a"""
        redis_graph.query(query)

        # # List of edge input should return list of NULL
        query = """CREATE (a:X), (b:Y)"""
        redis_graph.query(query)
        query = """MATCH (a:X), (b:Y) CREATE (a)-[e:R]->(b)"""
        redis_graph.query(query)
        query = """MATCH ()-[e]->() RETURN toBooleanList([e])"""
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

    def test05_toFloatList(self):
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

        try:
            redis_graph.query("RETURN toFloatList(true)")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch: expected List but was Boolean", str(e))

        try:
            redis_graph.query("RETURN toFloatList(7.05)")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch: expected List but was Float", str(e))

        try:
            redis_graph.query("RETURN toFloatList(7)")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch: expected List but was Integer", str(e))

        try:
            redis_graph.query("RETURN toFloatList('abc')")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch: expected List but was String", str(e))

        try:
            query = """CREATE (a:X)"""
            redis_graph.query(query)
            query = """MATCH (n) RETURN toFloatList(n)"""
            actual_result = redis_graph.query(query)
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch: expected List but was Node", str(e))
            query = """MATCH (a:X) DELETE a"""
            redis_graph.query(query)

        try:
            query = """CREATE (a:X), (b:Y)"""
            redis_graph.query(query)
            query = """MATCH (a:X), (b:Y) create (a)-[r:R]->(b)"""
            redis_graph.query(query)
            query = """MATCH ()-[e]->() RETURN toFloatList(e)"""
            actual_result = redis_graph.query(query)
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch: expected List but was Edge", str(e))
            query = """MATCH (a:X),(b:Y) DELETE a, b"""
            redis_graph.query(query)

        try:
            query = """CREATE (a:X), (b:Y)"""
            redis_graph.query(query)
            query = """MATCH (a:X), (b:Y) CREATE (a)-[r:R]->(b)"""
            redis_graph.query(query)
            query = """MATCH p=()-[]->() RETURN toFloatList(p)"""
            actual_result = redis_graph.query(query)
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch: expected List but was Path", str(e))
            query = """MATCH (a:X),(b:Y) DELETE a, b"""
            redis_graph.query(query)

        # List of node input should return list of NULL
        query = """CREATE (a:X)"""
        redis_graph.query(query)
        query = """MATCH (n) RETURN toFloatList([n])"""
        expected_result = [[None]]
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0], expected_result)
        query = """MATCH (a:X) DELETE a"""
        redis_graph.query(query)

        # List of edge input should return list of NULL
        query = """CREATE (a:X), (b:Y)"""
        redis_graph.query(query)
        query = """MATCH (a:X), (b:Y) CREATE (a)-[e:R]->(b)"""
        redis_graph.query(query)
        query = """MATCH ()-[e]->() RETURN toFloatList([e])"""
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

    def test06_toIntegerList(self):
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

        try:
            redis_graph.query("RETURN toIntegerList(true)")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch: expected List but was Boolean", str(e))

        try:
            redis_graph.query("RETURN toIntegerList(7.05)")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch: expected List but was Float", str(e))

        try:
            redis_graph.query("RETURN toIntegerList(7)")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch: expected List but was Integer", str(e))

        try:
            redis_graph.query("RETURN toIntegerList('abc')")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch: expected List but was String", str(e))

        try:
            query = """CREATE (a:X)"""
            redis_graph.query(query)
            query = """MATCH (n) RETURN toIntegerList(n)"""
            actual_result = redis_graph.query(query)
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch: expected List but was Node", str(e))
            query = """MATCH (a:X) DELETE a"""
            redis_graph.query(query)

        try:
            query = """CREATE (a:X), (b:Y)"""
            redis_graph.query(query)
            query = """MATCH (a:X), (b:Y) create (a)-[r:R]->(b)"""
            redis_graph.query(query)
            query = """MATCH ()-[e]->() RETURN toIntegerList(e)"""
            actual_result = redis_graph.query(query)
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch: expected List but was Edge", str(e))
            query = """MATCH (a:X), (b:Y) DELETE a, b"""
            redis_graph.query(query)

        try:
            query = """CREATE (a:X), (b:Y)"""
            redis_graph.query(query)
            query = """MATCH (a:X), (b:Y) CREATE (a)-[r:R]->(b)"""
            redis_graph.query(query)
            query = """MATCH p=()-[]->() RETURN toIntegerList(p)"""
            actual_result = redis_graph.query(query)
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch: expected List but was Path", str(e))
            query = """MATCH (a:X),(b:Y) DELETE a, b"""
            redis_graph.query(query)

        # List of node input should return list of NULL
        query = """CREATE (a:X)"""
        redis_graph.query(query)
        query = """MATCH (n) RETURN toIntegerList([n])"""
        expected_result = [[None]]
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0], expected_result)
        query = """MATCH (a:X) DELETE a"""
        redis_graph.query(query)

        # # List of edge input should return list of NULL
        query = """CREATE (a:X), (b:Y)"""
        redis_graph.query(query)
        query = """MATCH (a:X), (b:Y) CREATE (a)-[e:R]->(b)"""
        redis_graph.query(query)
        query = """MATCH ()-[e]->() RETURN toIntegerList([e])"""
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

    def test07_toStringList(self):
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

        try:
            redis_graph.query("RETURN toStringList(true)")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch: expected List but was Boolean", str(e))

        try:
            redis_graph.query("RETURN toStringList(7.05)")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch: expected List but was Float", str(e))

        try:
            redis_graph.query("RETURN toStringList(7)")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch: expected List but was Integer", str(e))

        try:
            redis_graph.query("RETURN toStringList('abc')")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch: expected List but was String", str(e))

        try:
            query = """CREATE (a:X)"""
            redis_graph.query(query)
            query = """MATCH (n) RETURN toStringList(n)"""
            actual_result = redis_graph.query(query)
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch: expected List but was Node", str(e))
            query = """MATCH (a:X) DELETE a"""
            redis_graph.query(query)

        try:
            query = """CREATE (a:X), (b:Y)"""
            redis_graph.query(query)
            query = """MATCH (a:X), (b:Y) create (a)-[r:R]->(b)"""
            redis_graph.query(query)
            query = """MATCH ()-[e]->() RETURN toStringList(e)"""
            actual_result = redis_graph.query(query)
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch: expected List but was Edge", str(e))
            query = """MATCH (a:X),(b:Y) DELETE a, b"""
            redis_graph.query(query)

        try:
            query = """CREATE (a:X), (b:Y)"""
            redis_graph.query(query)
            query = """MATCH (a:X), (b:Y) CREATE (a)-[r:R]->(b)"""
            redis_graph.query(query)
            query = """MATCH p=()-[]->() RETURN toStringList(p)"""
            actual_result = redis_graph.query(query)
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Type mismatch: expected List but was Path", str(e))
            query = """MATCH (a:X),(b:Y) DELETE a, b"""
            redis_graph.query(query)

        # List of node input should return list of NULL
        query = """CREATE (a:X)"""
        redis_graph.query(query)
        query = """MATCH (n) RETURN toStringList([n])"""
        expected_result = [[None]]
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0], expected_result)
        query = """MATCH (a:X) DELETE a"""
        redis_graph.query(query)

        # # List of edge input should return list of NULL
        query = """CREATE (a:X), (b:Y)"""
        redis_graph.query(query)
        query = """MATCH (a:X), (b:Y) CREATE (a)-[e:R]->(b)"""
        redis_graph.query(query)
        query = """MATCH ()-[e]->() RETURN toStringList([e])"""
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
