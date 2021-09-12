import sys
import os
from RLTest import Env
from base import FlowTestsBase
from redis import ResponseError
from redisgraph import Graph

redis_con = None
redis_graph = None

class testQueryTimeout(FlowTestsBase):
    def __init__(self):
        # skip test if we're running under Valgrind
        if Env().envRunner.debugger is not None or os.getenv('COV') == '1':
            Env().skip() # queries will be much slower under Valgrind

        self.env = Env(decodeResponses=True)
        global redis_con
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph("timeout", redis_con)

    def test01_read_query_timeout(self):
        query = "UNWIND range(0,100000) AS x WITH x AS x WHERE x = 10000 RETURN x"
        try:
            # The query is expected to timeout
            redis_graph.query(query, timeout=1)
            assert(False)
        except ResponseError as error:
            self.env.assertContains("Query timed out", str(error))

        try:
            # The query is expected to succeed
            redis_graph.query(query, timeout=100)
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
        query = "UNWIND range(0,100000) AS x WITH x AS x WHERE x = 10000 RETURN x"
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
        query = """UNWIND range(0, 100000) AS x CREATE (p:Person {age: x%90, height: x%200, weight: x%80})"""
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
                redis_graph.query(q, timeout=10)
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

