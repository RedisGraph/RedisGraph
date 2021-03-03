import sys
from RLTest import Env
from base import FlowTestsBase
from redis import ResponseError
from redisgraph import Graph

redis_con = None
redis_graph = None

class testQueryTimeout(FlowTestsBase):
    def __init__(self):
        # skip test if we're running under Valgrind
        if Env().envRunner.debugger is not None:
            Env().skip() # queries will be much slower under Valgrind

        self.env = Env(decodeResponses=True)
        global redis_con
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph("timeout", redis_con)

    def test01_read_query_timeout(self):
        query = "UNWIND range(0,100000) AS x WITH x AS x WHERE x = 10000 RETURN x"
        try:
            # The query is expected to time out
            redis_graph.query(query, timeout=1)
            assert(False)
        except ResponseError as error:
            self.env.assertContains("Query timed out", str(error))

        try:
            # The query is expected to succeed
            redis_graph.query(query, timeout=100)
        except:
            assert(False)

    def test02_write_query_timeout(self):
        query = "UNWIND range(0,100000) AS x WITH x AS x WHERE x = 10000 CREATE (:L)"
        try:
            redis_graph.query(query, timeout=1)
            assert(False)
        except ResponseError as error:
            self.env.assertContains("Query timed out", str(error))
        # Validate that no data was created
        query = "MATCH (a:L) RETURN a"
        actual_result = redis_graph.query(query)
        expected_result = []
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = "CREATE (a:L {v: 1}) RETURN a.v"
        try:
            # The query is expected to succeed
            actual_result = redis_graph.query(query, timeout=100)
            # Validate that data was created
            expected_result = [[1]]
            self.env.assertEquals(actual_result.result_set, expected_result)
        except:
            assert(False)

    def test03_configured_timeout(self):
        # Set a default timeout of 1 millisecond
        redis_con.execute_command("GRAPH.CONFIG SET timeout 1")

        # Validate that a read query times out
        query = "UNWIND range(0,100000) AS x WITH x AS x WHERE x = 10000 RETURN x"
        try:
            redis_graph.query(query)
            assert(False)
        except ResponseError as error:
            self.env.assertContains("Query timed out", str(error))

        # Validate that a write query times out
        query = "UNWIND range(0,100000) AS x WITH x AS x WHERE x = 10000 CREATE (:L)"
        try:
            redis_graph.query(query)
            assert(False)
        except ResponseError as error:
            self.env.assertContains("Query timed out", str(error))
