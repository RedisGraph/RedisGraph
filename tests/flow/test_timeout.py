from common import *

redis_con = None
redis_graph = None


class testQueryTimeout(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)
        # skip test if we're running under Valgrind
        if self.env.envRunner.debugger is not None or os.getenv('COV') == '1':
            self.env.skip() # queries will be much slower under Valgrind

        global redis_con
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(redis_con, "timeout")

    def test01_read_query_timeout(self):
        query = "UNWIND range(0,1000000) AS x WITH x AS x WHERE x = 10000 RETURN x"
        try:
            # The query is expected to timeout
            redis_graph.query(query, timeout=1)
            assert(False)
        except ResponseError as error:
            self.env.assertContains("Query timed out", str(error))

        try:
            # The query is expected to succeed
            redis_graph.query(query, timeout=2000)
        except:
            assert(False)

    def test02_configured_timeout(self):
        # Verify that the module-level timeout is set to the default of 0
        response = redis_con.execute_command("GRAPH.CONFIG GET timeout")
        self.env.assertEquals(response[1], 0)
        # Set a default timeout of 1 millisecond
        redis_con.execute_command("GRAPH.CONFIG SET timeout 1")
        response = redis_con.execute_command("GRAPH.CONFIG GET timeout")
        self.env.assertEquals(response[1], 1)

        # Validate that a read query times out
        query = "UNWIND range(0,1000000) AS x WITH x AS x WHERE x = 10000 RETURN x"
        try:
            redis_graph.query(query)
            assert(False)
        except ResponseError as error:
            self.env.assertContains("Query timed out", str(error))

    def test03_write_query_ignore_timeout(self):
        #----------------------------------------------------------------------
        # verify that the timeout argument is ignored by write queries
        #----------------------------------------------------------------------
        write_queries = [
            # create query
            "UNWIND range(0, 10000) AS x CREATE (a:M)",
            # update query
            "MATCH (a:M) SET a.v = 2",
            # delete query
            "MATCH (a:M) DELETE a"
        ]

        # queries should complete successfully
        for q in write_queries:
            try:
                result = redis_graph.query(q, timeout=1)
                # the query should have taken longer than the timeout value
                self.env.assertGreater(result.run_time_ms, 2)
            except ResponseError:
                assert(False)

        #----------------------------------------------------------------------
        # index creation should ignore timeouts
        #----------------------------------------------------------------------
        query = "UNWIND range (0, 100000) AS x CREATE (:M {v:x})"
        redis_graph.query(query)

        # create index
        query = "CREATE INDEX ON :M(v)"
        try:
            # the query should complete successfully
            result = redis_graph.query(query, timeout=1)
            self.env.assertEquals(result.indices_created, 1)
        except ResponseError:
            assert(False)

        #----------------------------------------------------------------------
        # index deletion should ignore timeouts
        #----------------------------------------------------------------------
        query = "DROP INDEX ON :M(v)"
        try:
            # the query should complete successfully
            result = redis_graph.query(query, timeout=1)
            self.env.assertEquals(result.indices_deleted, 1)
        except ResponseError:
            assert(False)

    def test04_timeout_index_scan(self):
        # construct a graph and create multiple indices
        query = """UNWIND range(0, 500000) AS x CREATE (p:Person {age: x%90, height: x%200, weight: x%80})"""
        redis_graph.query(query)

        query = """CREATE INDEX ON :Person(age, height, weight)"""
        redis_graph.query(query)

        queries = [
                # full scan
                "MATCH (a) RETURN a",
                # ID scan
                "MATCH (a) WHERE ID(a) > 20 RETURN a",
                # label scan
                "MATCH (a:Person) RETURN a",
                # single index scan
                "MATCH (a:Person) WHERE a.age > 40 RETURN a",
                # index scan + full scan
                "MATCH (a:Person), (b) WHERE a.age > 40 RETURN a, b",
                # index scan + ID scan
                "MATCH (a:Person), (b) WHERE a.age > 40 AND ID(b) > 20 RETURN a, b",
                # index scan + label scan
                "MATCH (a:Person), (b:Person) WHERE a.age > 40 RETURN a, b",
                # multi full and index scans
                "MATCH (a:Person), (b:Person), (c), (d) WHERE a.age > 40 AND b.height < 150 RETURN a,b,c,d",
                # multi ID and index scans
                "MATCH (a:Person), (b:Person), (c:Person), (d) WHERE a.age > 40 AND b.height < 150 AND ID(c) > 20 AND ID(d) > 30 RETURN a,b,c,d",
                # multi label and index scans
                "MATCH (a:Person), (b:Person), (c:Person), (d:Person) WHERE a.age > 40 AND b.height < 150 RETURN a,b,c,d",
                # multi index scans
                "MATCH (a:Person), (b:Person), (c:Person) WHERE a.age > 40 AND b.height < 150 AND c.weight = 50 RETURN a,b,c"
                ]

        for q in queries:
            try:
                # query is expected to timeout
                redis_graph.query(q, timeout=1)
                assert(False)
            except ResponseError as error:
                self.env.assertContains("Query timed out", str(error))

        # validate that server didn't crash
        redis_con.ping()

        # rerun each query with timeout and limit
        # expecting queries to run to completion
        for q in queries:
            q += " LIMIT 2"
            redis_graph.query(q, timeout=10)

        # validate that server didn't crash
        redis_con.ping()

    def test05_query_timeout_free_resultset(self):
        query = "UNWIND range(0,1000000) AS x RETURN toString(x)"
        try:
            # The query is expected to timeout
            redis_graph.query(query, timeout=10)
            assert(False)
        except ResponseError as error:
            self.env.assertContains("Query timed out", str(error))

        try:
            # The query is expected to succeed
            redis_graph.query(query, timeout=2000)
        except:
            self.env.assertTrue(True)

    def test06_error_timeout_default_higher_than_timeout_max(self):
        self.env.stop()
        self.env = Env(decodeResponses=True, moduleArgs="TIMEOUT_DEFAULT 10 TIMEOUT_MAX 10")

        # get current timeout configuration
        max_timeout = redis_con.execute_command("GRAPH.CONFIG", "GET", "TIMEOUT_MAX")[1]
        default_timeout = redis_con.execute_command("GRAPH.CONFIG", "GET", "TIMEOUT_DEFAULT")[1]

        self.env.assertEquals(max_timeout, 10)
        self.env.assertEquals(default_timeout, 10)

        # try to set default-timeout to a higher value than max-timeout
        try:
            redis_con.execute_command("GRAPH.CONFIG", "SET", "TIMEOUT_DEFAULT", max_timeout + 1)
            self.env.assertTrue(False)
        except ResponseError as error:
            self.env.assertContains("TIMEOUT_DEFAULT configuration parameter cannot be set to a value higher than TIMEOUT_MAX", str(error))

        # try to set max-timeout to a lower value then default-timeout
        try:
            redis_con.execute_command("GRAPH.CONFIG", "SET", "TIMEOUT_MAX", default_timeout - 1)
            self.env.assertTrue(False)
        except ResponseError as error:
            self.env.assertContains("TIMEOUT_MAX configuration parameter cannot be set to a value lower than TIMEOUT_DEFAULT", str(error))

        # disable max timeout
        try:
            redis_con.execute_command("GRAPH.CONFIG", "SET", "TIMEOUT_MAX", 0)
            self.env.assertTrue(True)
            # revert timeout_max to 10
            redis_con.execute_command("GRAPH.CONFIG", "SET", "TIMEOUT_MAX", 10)
        except ResponseError as error:
            self.env.assertTrue(False)

        # disable default timeout
        try:
            redis_con.execute_command("GRAPH.CONFIG", "SET", "TIMEOUT_DEFAULT", 0)
            self.env.assertTrue(True)
            # revert timeout_default to 10
            redis_con.execute_command("GRAPH.CONFIG", "SET", "TIMEOUT_DEFAULT", 10)
        except ResponseError as error:
            self.env.assertTrue(False)

    def test07_read_write_query_timeout_default(self):
        queries = [
            "UNWIND range(0,1000000) AS x WITH x AS x WHERE x = 10000 RETURN x",
            "UNWIND range(0,1000000) AS x CREATE (:N {v: x})"
        ]

        for _ in range(1, 2):
            for query in queries:
                try:
                    # The query is expected to timeout
                    redis_graph.query(query)
                    self.env.assertTrue(False)
                except ResponseError as error:
                    self.env.assertContains("Query timed out", str(error))

            # disable timeout_default, timeout_max should be enforced
            redis_con.execute_command("GRAPH.CONFIG", "SET", "TIMEOUT_DEFAULT", 0)
        
        # revert timeout_default to 10
        redis_con.execute_command("GRAPH.CONFIG", "SET", "TIMEOUT_DEFAULT", 10)

    def test08_enforce_timeout_configuration(self):
        read_q = "RETURN 1"
        write_q = "CREATE ()"
        queries = [read_q, write_q]

        max_timeout = redis_con.execute_command("GRAPH.CONFIG", "GET", "TIMEOUT_MAX")[1]

        for query in queries:
            try:
                # query is expected to fail
                redis_graph.query(query, timeout=max_timeout+1)
                self.env.assertTrue(False)
            except ResponseError as error:
                self.env.assertContains("The query TIMEOUT parameter value cannot exceed the TIMEOUT_MAX configuration parameter value", str(error))

    def test_09_fallback(self):
        self.env.stop()
        self.env = Env(decodeResponses=True, moduleArgs="TIMEOUT 1")

        configs = ["TIMEOUT_DEFAULT", "TIMEOUT_MAX"]

        for config in configs:
            # enable/disable config expecting to fallback to the old timeout
            redis_con.execute_command("GRAPH.CONFIG", "SET", config, 10)
            redis_con.execute_command("GRAPH.CONFIG", "SET", config, 0)

            query = "UNWIND range(0,1000000) AS x WITH x AS x WHERE x = 10000 RETURN x"
            try:
                # The query is expected to timeout
                redis_graph.query(query)
                self.env.assertTrue(False)
            except ResponseError:
                self.env.assertTrue(True)

            query = "UNWIND range(0, 1000000) AS x CREATE (:N {v: x})"
            try:
                # The query is expected to succeed
                redis_graph.query(query)
                self.env.assertTrue(True)
            except:
                self.env.assertTrue(False)

    def test09_set_old_timeout_when_new_config_set(self):
        redis_con.execute_command("GRAPH.CONFIG", "SET", "TIMEOUT_DEFAULT", 10)

        # try to set timeout
        try:
            redis_con.execute_command("GRAPH.CONFIG", "SET", "TIMEOUT", 20)
            self.env.assertTrue(False)
        except ResponseError as error:
            self.env.assertContains("The TIMEOUT configuration parameter is deprecated. Please set TIMEOUT_MAX and TIMEOUT_DEFAULT instead", str(error))

    # When timeout occurs while executing a PROFILE command, only the error-message
    # should return to user
    def test10_profile_no_double_response(self):
        # reset timeout params to default
        self.env.stop()
        self.env = Env(decodeResponses=True)

        # Set timeout parameters to small values (1 millisecond)
        redis_graph.config("TIMEOUT_MAX", 1, True)
        redis_graph.config("TIMEOUT_DEFAULT", 1, True)

        # Issue a profile query, expect a timeout error
        try:
            redis_graph.profile("UNWIND range(0, 100000000) AS x CREATE (:P{v:x})")
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertTrue(True)
        
        # make sure no pending result exists
        res = redis_graph.query("RETURN 1")
        self.env.assertEquals(res.result_set[0][0], 1)
