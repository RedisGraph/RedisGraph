from common import *
import re

redis_graph = None
GRAPH_ID = "unwind"

class testUnwindClause():
    
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(redis_con, GRAPH_ID)
 
    def test01_unwind_null(self):
        query = """UNWIND null AS x RETURN x"""
        actual_result = redis_graph.query(query)
        expected = []
        self.env.assertEqual(actual_result.result_set, expected)

    def test02_unwind_input_types(self):
        # map list input
        query = """UNWIND ([{x:3, y:5}]) AS q RETURN q"""
        actual_result = redis_graph.query(query)
        expected = [[{'x':3, 'y':5}]]
        self.env.assertEqual(actual_result.result_set, expected)

        # map input
        query = """UNWIND ({x:3, y:5}) AS q RETURN q"""
        actual_result = redis_graph.query(query)
        expected = [[{'x': 3, 'y': 5}]]
        self.env.assertEqual(actual_result.result_set, expected)

        # map containing a key with the value NULL
        query = """UNWIND ({x:null}) AS q RETURN q"""
        actual_result = redis_graph.query(query)
        expected = [[{'x': None}]]
        self.env.assertEqual(actual_result.result_set, expected)

        # integer input
        query = """UNWIND 5 AS q RETURN q"""
        actual_result = redis_graph.query(query)
        expected = [[5]]
        self.env.assertEqual(actual_result.result_set, expected)

        # string input
        query = """UNWIND 'abc' AS q RETURN q"""
        actual_result = redis_graph.query(query)
        expected = [['abc']]
        self.env.assertEqual(actual_result.result_set, expected)

        # floating-point input
        query = """UNWIND 7.5 AS q RETURN q"""
        actual_result = redis_graph.query(query)
        expected = [[7.5]]
        self.env.assertEqual(actual_result.result_set, expected)

        # nested list
        query = """WITH [[1, 2], [3, 4], 5] AS nested UNWIND nested AS x RETURN x"""
        actual_result = redis_graph.query(query)
        expected = [[[1, 2]], [[3, 4]], [5]]
        self.env.assertEqual(actual_result.result_set, expected)

        # nested list double unwind
        query = """WITH [[1, 2], [3, 4], 5] AS nested UNWIND nested AS x UNWIND x AS y RETURN y"""
        actual_result = redis_graph.query(query)
        expected = [[1], [2], [3], [4], [5]]
        self.env.assertEqual(actual_result.result_set, expected)

        # empty list
        query = """UNWIND [] AS x RETURN x"""
        actual_result = redis_graph.query(query)
        expected = []
        self.env.assertEqual(actual_result.result_set, expected)

        # list with null at the last position
        query = """UNWIND [1, 2, null] AS x RETURN x"""
        actual_result = redis_graph.query(query)
        expected = [[1], [2], [None]]
        self.env.assertEqual(actual_result.result_set, expected)

        # list with null before the last position
        query = """UNWIND [1, null, 2] AS x RETURN x"""
        actual_result = redis_graph.query(query)
        expected = [[1], [None], [2]]
        self.env.assertEqual(actual_result.result_set, expected)

        # list with null at first position
        query = """UNWIND [null, 1, 2] AS x RETURN x"""
        actual_result = redis_graph.query(query)
        expected = [[None], [1], [2]]
        self.env.assertEqual(actual_result.result_set, expected)

    def test02_unwind_set(self):
        # delete property
        query = """CREATE (n:N {x:3})"""
        actual_result = redis_graph.query(query)
        query = """UNWIND ({x:null}) AS q MATCH (n:N) SET n.x= q.x RETURN n"""
        actual_result = redis_graph.query(query)
        self.env.assertEqual(actual_result.properties_removed, 1)
    
    def test03_unwind_range(self):
        query = """UNWIND range(1, 3) AS x RETURN x"""
        actual_result = redis_graph.query(query)
        self.env.assertEqual(actual_result.result_set, [[1], [2], [3]])

        query = """UNWIND range(3, 1) AS x RETURN x"""
        actual_result = redis_graph.query(query)
        self.env.assertEqual(actual_result.result_set, [])

        query = """UNWIND range(3, 1, -1) AS x RETURN x"""
        actual_result = redis_graph.query(query)
        self.env.assertEqual(actual_result.result_set, [[3], [2], [1]])

        query = """UNWIND range(1, 3, -1) AS x RETURN x"""
        actual_result = redis_graph.query(query)
        self.env.assertEqual(actual_result.result_set, [])

        query = """WITH 1 AS from UNWIND range(from, 3) AS x RETURN x"""
        actual_result = redis_graph.query(query)
        self.env.assertEqual(actual_result.result_set, [[1], [2], [3]])

        query = """WITH 1.2 AS from UNWIND range(from, 3) AS x RETURN x"""
        try:
            redis_graph.query(query)
        except ResponseError as e:
            self.env.assertContains("Type mismatch: expected Integer but was Float", str(e))

        query = """WITH 1.2 AS to UNWIND range(1, to) AS x RETURN x"""
        try:
            redis_graph.query(query)
        except ResponseError as e:
            self.env.assertContains("Type mismatch: expected Integer but was Float", str(e))

        query = """WITH 1.2 AS step UNWIND range(1, 3, step) AS x RETURN x"""
        try:
            redis_graph.query(query)
        except ResponseError as e:
            self.env.assertContains("Type mismatch: expected Integer but was Float", str(e))

