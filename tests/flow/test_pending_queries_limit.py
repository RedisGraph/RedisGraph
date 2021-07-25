from RLTest import Env
from redisgraph import Graph
import utils.multiproc as mlp
from utils.multiproc import *

# 1.test getting and setting config
# 2. test overflowing the server when there's a limit
#    expect to get error!
# 3. test overflowing the server when there's no limit
#    expect not to get any exceptions

GRAPH_NAME = "max_pending_queries"
SLOW_QUERY = "UNWIND range (0, 100000) AS x WITH x WHERE (x / 2) = 50  RETURN x"

class testPendingQueryLimit():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        self.conn = self.env.getConnection()

    def test_01_query_limit_config(self):
        # read max queued queries config
        result = self.conn.execute_command("GRAPH.CONFIG", "GET", "MAX_QUEUED_QUERIES")
        max_queued_queries = result[1]
        self.env.assertEquals(max_queued_queries, 4294967295)

        # update configuration, set max queued queries
        self.conn.execute_command("GRAPH.CONFIG", "SET", "MAX_QUEUED_QUERIES", 10)

        # re-read configuration
        result = self.conn.execute_command("GRAPH.CONFIG", "GET", "MAX_QUEUED_QUERIES")
        max_queued_queries = result[1]
        self.env.assertEquals(max_queued_queries, 10)

    def stress_server(self, should_fail):
        threadpool_size = self.conn.execute_command("GRAPH.CONFIG", "GET", "THREAD_COUNT")[1]
        nprocs = threadpool_size * 5
        failed = False
        res = mlp.run_queries_multiproc(self.env, [(SLOW_QUERY,)]*nprocs, [(GRAPH_NAME,)]*nprocs, 1)
        for r in res:
            if isinstance(r, Exception):
                assert "Max pending queries exceeded" in str(r)
                self.env.assertTrue(should_fail)
                failed = True

        self.env.assertEqual(failed, should_fail)

    def test_02_overflow_no_limit(self):

        # no limit on number of pending queries
        limit = 4294967295
        self.conn.execute_command("GRAPH.CONFIG", "SET", "MAX_QUEUED_QUERIES", limit)

        self.stress_server(False)

    def test_03_overflow_with_limit(self):

        # limit number of pending queries
        limit = 1
        self.conn.execute_command("GRAPH.CONFIG", "SET", "MAX_QUEUED_QUERIES", limit)

        self.stress_server(True)

