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
                assert(False)
            except ResponseError as error:
                self.env.assertContains("Query timed out", str(error))

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
            assert(False)
