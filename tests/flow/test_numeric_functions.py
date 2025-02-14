from cmath import isinf, isnan
from common import *
import math

redis_graph = None
GRAPH_ID = "numeric"

class testNumericFunctions():
    
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

    def expect_type_error(self, query):
        self.expect_error(query, "Type mismatch")
    
    def get_res_and_assertEquals(self, query, expected_result):
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

    def test01_NanLiteral(self):
        query = "RETURN NaN"
        actual_result = redis_graph.query(query)
        self.env.assertTrue(math.isnan(actual_result.result_set[0][0]))

        query = "RETURN -1*NaN"
        actual_result = redis_graph.query(query)
        self.env.assertTrue(math.isnan(actual_result.result_set[0][0]))

        query = "RETURN -1/NaN"
        actual_result = redis_graph.query(query)
        self.env.assertTrue(math.isnan(actual_result.result_set[0][0]))

        query = "CREATE (n:N {a:Nan}) RETURN n.a"
        actual_result = redis_graph.query(query)
        self.env.assertTrue(math.isnan(actual_result.result_set[0][0]))

    def test02_InfLiteral(self):
        query = "RETURN Inf"
        actual_result = redis_graph.query(query)
        self.env.assertTrue(math.isinf(actual_result.result_set[0][0]))

        query = "CREATE (n:N {a:Inf}) RETURN n.a"
        actual_result = redis_graph.query(query)
        self.env.assertTrue(math.isinf(actual_result.result_set[0][0]))

        query = "RETURN Infinity"
        actual_result = redis_graph.query(query)
        self.env.assertTrue(math.isinf(actual_result.result_set[0][0]))

        query = "CREATE (n:N {a:Infinity}) RETURN n.a"
        actual_result = redis_graph.query(query)
        self.env.assertTrue(math.isinf(actual_result.result_set[0][0]))

    def test03_IsNan(self):
        query_to_expected_result = {
            "RETURN IsNan(Nan)" : [[True]],
            "RETURN IsNan(-Nan)" : [[True]],
            "RETURN IsNan(NULL)" : [[None]],
            "RETURN IsNan(0/0.0)" : [[True]],
            "RETURN IsNan(0%0.0)" : [[True]],
            "RETURN IsNan(1/0.0)" : [[False]],
            "RETURN IsNan(Inf)" : [[False]],
            "RETURN IsNan(-Inf)" : [[False]],
            "RETURN IsNan(-1234)" : [[False]],
            "RETURN IsNan(0.4878)" : [[False]],
            "RETURN IsNan(Infinity)" : [[False]],
            
        }
        for query, expected_result in query_to_expected_result.items():
            self.get_res_and_assertEquals(query, expected_result)

        # invalid input types
        queries = [
            "RETURN IsNan('a')",
            "RETURN IsNan(true)",
            "CREATE (n:N) RETURN IsNan(n)",
            "CREATE ()-[r:R]->() RETURN IsNan(r)",
        ]
        for query in queries:
            self.expect_type_error(query)
        

        