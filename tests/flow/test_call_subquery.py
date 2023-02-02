from common import *
from collections import Counter

graph = None
GRAPH_ID = "G"

class testCallSubqueryFlow():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global graph
        redis_con = self.env.getConnection()
        graph = Graph(redis_con, GRAPH_ID)
    
    def get_res_and_assertEquals(self, query, expected_result):
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)
        return actual_result

    # validate the non-valid queries don't pass validations
    def test01_test_validations(self):
        # non-simple imports
        for query in [
            "WITH 1 AS a CALL {WITH 1 AS a RETURN 1} RETURN 1",
            "WITH 1 AS a CALL {WITH a+1 AS a RETURN a} RETURN a",
            "WITH 1 AS a CALL {WITH a AS b RETURN b} RETURN b",
            "WITH 1 AS a CALL {WITH a LIMIT 5 RETURN a} RETURN a",
            "WITH 1 AS a CALL {WITH a ORDER BY a.v RETURN a} RETURN a",
            "WITH 1 AS a CALL {WITH a WHERE a > 5 RETURN a} RETURN a",
            "WITH 1 AS a CALL {WITH a SKIP 5 RETURN a} RETURN a",
        ]:
            try:
                graph.query(query)
            except redis.exceptions.ResponseError as e:
                self.env.assertIn("WITH imports in CALL {} must be simple ('WITH a')",
                    str(e))

        # non-valid queries within CAll {}
        for query in [
            "CALL {CREATE (n:N) MATCH (n:N) RETURN n} RETURN 1",
            "WITH 1 AS a CALL {WITH a CREATE (n:N) MATCH (n:N) RETURN n} RETURN a",
            "CALL {MATCH (n:N) CREATE (n:N2)} RETURN 1"
        ]:
            try:
                graph.query(query)
                self.env.assertTrue(False)
            except redis.exceptions.ResponseError as e:
                pass
