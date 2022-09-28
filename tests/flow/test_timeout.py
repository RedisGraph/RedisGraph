from common import *

redis_con = None
redis_graph = None

GRAPH_ID = "timeout"

class testQueryTimeout():
    def __init__(self):
        self.env = Env(decodeResponses=True, moduleArgs="TIMEOUT 1000")
        # skip test if we're running under Valgrind
        if self.env.envRunner.debugger is not None or os.getenv('COV') == '1':
            self.env.skip() # queries will be much slower under Valgrind

        global redis_con
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(redis_con, GRAPH_ID)

    def test01_read_write_query_timeout(self):
        query = "UNWIND range(0,1000000) AS x WITH x AS x WHERE x = 10000 RETURN x"
        try:
            # The query is expected to timeout
            redis_graph.query(query, timeout=1)
            self.env.assertTrue(False)
        except ResponseError as error:
            self.env.assertContains("Query timed out", str(error))

        try:
            # The query is expected to succeed
            redis_graph.query(query, timeout=2000)
        except:
            self.env.assertTrue(False)

        query = "UNWIND range(0, 1000000) AS x CREATE (:N {v: x})"
        try:
            # The query is expected to succeed
            redis_graph.query(query, timeout=1)
            self.env.assertTrue(True)
        except:
            self.env.assertTrue(False)

    def test02_configured_timeout(self):
        # Verify that the module-level timeout is set to the default of 0
        response = redis_con.execute_command("GRAPH.CONFIG GET timeout")
        self.env.assertEquals(response[1], 1000)
        # Set a default timeout of 1 millisecond
        redis_con.execute_command("GRAPH.CONFIG SET timeout 1")
        response = redis_con.execute_command("GRAPH.CONFIG GET timeout")
        self.env.assertEquals(response[1], 1)

        # Validate that a read query times out
        query = "UNWIND range(0,1000000) AS x WITH x AS x WHERE x = 10000 RETURN x"
        try:
            redis_graph.query(query)
            self.env.assertTrue(False)
        except ResponseError as error:
            self.env.assertContains("Query timed out", str(error))

    def test03_timeout_index_scan(self):
        # set timeout to unlimited
        redis_con.execute_command("GRAPH.CONFIG SET timeout 0")

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
                self.env.assertTrue(False)
            except ResponseError as error:
                self.env.assertContains("Query timed out", str(error))

        # rerun each query with timeout and limit
        # expecting queries to run to completion
        for q in queries:
            q += " LIMIT 2"
            res = redis_graph.query(q, timeout=20)

    def test04_query_timeout_free_resultset(self):
        query = "UNWIND range(0,1000000) AS x RETURN toString(x)"
        try:
            # The query is expected to timeout
            redis_graph.query(query, timeout=10)
            self.env.assertTrue(False)
        except ResponseError as error:
            self.env.assertContains("Query timed out", str(error))

        try:
            # The query is expected to succeed
            redis_graph.query(query, timeout=2000)
        except:
            self.env.assertTrue(False)

    # def test05_ignore_timeout_when_timeout_max_or_timeout_default_set(self):
    #     self.env.stop()
    #     self.env = Env(decodeResponses=True, moduleArgs="TIMEOUT 10 TIMEOUT_DEFAULT 10 TIMEOUT_MAX 10")

    #     logfilename = self.env.envRunner._getFileName("master", ".log")
    #     logfile = open(f"{self.env.logDir}/{logfilename}")
    #     log = logfile.read()

    #     self.env.assertContains("The deprecated TIMEOUT configuration parameter is ignored. Please remove it from the configuration file", log)

    #     self.env = Env(decodeResponses=True, moduleArgs="TIMEOUT_DEFAULT 10 TIMEOUT_MAX 10")

    def test06_error_timeout_default_higher_than_timeout_max(self):
        self.env.stop()
        self.env = Env(decodeResponses=True, moduleArgs="TIMEOUT_DEFAULT 10 TIMEOUT_MAX 10")

        # get current timeout configuration
        max_timeout = redis_con.execute_command("GRAPH.CONFIG", "GET", "TIMEOUT_MAX")[1]
        default_timeout = redis_con.execute_command("GRAPH.CONFIG", "GET", "TIMEOUT_DEFAULT")[1]

        # try to set default-timeout to a higher value than max-timeout
        try:
            redis_con.execute_command("GRAPH.CONFIG", "SET", "TIMEOUT_DEFAULT", max_timeout + 1)
            self.env.assertTrue(False)
        except ResponseError as error:
            self.env.assertContains("Failed to set config value TIMEOUT_DEFAULT", str(error))

        # try to set max-timout to a lower value then default-timout
        try:
            redis_con.execute_command("GRAPH.CONFIG", "SET", "TIMEOUT_MAX", default_timeout - 1)
            self.env.assertTrue(False)
        except ResponseError as error:
                self.env.assertContains("Failed to set config value TIMEOUT_MAX", str(error))

    def test07_read_write_query_timeout_default(self):
        queries = [
            "UNWIND range(0,1000000) AS x WITH x AS x WHERE x = 10000 RETURN x",
            "UNWIND range(0,1000000) AS x CREATE (:N {v: x})"
        ]
        for query in queries:
            try:
                # The query is expected to timeout
                redis_graph.query(query)
                self.env.assertTrue(False)
            except ResponseError as error:
                self.env.assertContains("Query timed out", str(error))

    def test08_enforce_timeout_configuration(self):
        read_q = "RETURN 1"
        write_q = "CREATE ()"
        queries = [read_q, write_q]

        for query in queries:
            try:
                # query is expected to fail
                redis_graph.query(query, timeout=2000)
                self.env.assertTrue(False)
            except ResponseError as error:
                self.env.assertContains("The query TIMEOUT parameter value cannot exceed the TIMEOUT_MAX configuration parameter value", str(error))

