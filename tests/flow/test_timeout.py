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
        # Verify that the module-level timeout is set to the default of 0
        response = redis_con.execute_command("GRAPH.CONFIG GET timeout")
        self.env.assertEquals(response[1], 0)
        # Set a default timeout of 1 millisecond
        redis_con.execute_command("GRAPH.CONFIG SET timeout 1")
        response = redis_con.execute_command("GRAPH.CONFIG GET timeout")
        self.env.assertEquals(response[1], 1)

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

    def test04_timeout_during_commit_stage(self):
        query = "CREATE (a:M) WITH a UNWIND range(1,10000) AS ctr SET a.v = ctr"
        try:
            # The query should complete successfully
            actual_result = redis_graph.query(query, timeout=1)
            # The query should have taken longer than the timeout value
            self.env.assertGreater(actual_result.run_time_ms, 1)
            # The query should have updated properties 10,000 times
            self.env.assertEquals(actual_result.properties_set, 10000)
        except ResponseError:
            assert(False)

        # Validate that the node retains its final updated value
        query = "MATCH (a:M) RETURN a.v"
        actual_result = redis_graph.query(query)
        expected_result = [[10000]]
        self.env.assertEquals(actual_result.result_set, expected_result)
