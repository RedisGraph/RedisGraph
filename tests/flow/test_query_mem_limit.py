from common import *
import random
from pathos.pools import ProcessPool as Pool

# 1. test reading and setting query memory limit configuration

# 2. test overflowing the server when there's no limit,
#    expect no errors

# 3. test querying the server when there's a high memory limit and the queries
#    are all running under the current limit

# 4. test querying the server when there's a tight memory limit
#    expecting an out of memory error

# 5. test a mixture of queries, ~90% successful ones and the rest are expected
#    to fail due to out of memory error

g                  =  None
GRAPH_NAME         =  "max_query_mem"
MEM_HOG_QUERY      =  """UNWIND range(0, 100000) AS x
                         WITH x
                         WHERE (x / 2) = 50
                         RETURN x, count(x)"""

MEM_THRIFTY_QUERY  =  """UNWIND range(0, 10) AS x
                         WITH x
                         WHERE (x / 2) = 50
                         RETURN x, count(x)"""


def issue_query(conn, q, should_fail):
    try:
        g = Graph(conn, GRAPH_NAME)
        g.query(q)
        return not should_fail
    except Exception as e:
        assert "Query's mem consumption exceeded capacity" in str(e)
        return should_fail

class testQueryMemoryLimit():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        # skip test if we're running under Valgrind
        if VALGRIND:
            self.env.skip() # valgrind is not working correctly with multi process

        self.conn = self.env.getConnection()

    def stress_server(self, queries):
        connections = []
        qs = []
        should_fails = []
        thread_count = self.conn.execute_command("GRAPH.CONFIG", "GET", "THREAD_COUNT")[1]
        pool = Pool(nodes=thread_count)

        # init connections
        for t in range(thread_count):
            connections.append(self.env.getConnection())

        for q in queries:
            qs.append(q[0])
            should_fails.append(q[1])

        # invoke queries
        result = pool.map(issue_query, connections, qs, should_fails)

        # validate all process return true
        self.env.assertTrue(all(result))

    def test_01_read_memory_limit_config(self):
        # read configuration, test default value, expecting unlimited memory cap
        result = self.conn.execute_command("GRAPH.CONFIG", "GET", "QUERY_MEM_CAPACITY")
        query_mem_capacity = result[1]
        self.env.assertEquals(query_mem_capacity, 0)

        # update configuration, set memory limit to 1MB
        MB = 1024*1024
        self.conn.execute_command("GRAPH.CONFIG", "SET", "QUERY_MEM_CAPACITY", MB)

        # re-read configuration
        result = self.conn.execute_command("GRAPH.CONFIG", "GET", "QUERY_MEM_CAPACITY")
        query_mem_capacity = result[1]
        self.env.assertEquals(query_mem_capacity, MB)

    def test_02_overflow_no_limit(self):
        # execute query on each one of the threads
        n_queries_to_execute = self.conn.execute_command("GRAPH.CONFIG", "GET", "THREAD_COUNT")[1]

        # set query memory limit as UNLIMITED
        limit = 0
        self.conn.execute_command("GRAPH.CONFIG", "SET", "QUERY_MEM_CAPACITY", limit) 

        self.stress_server([(MEM_HOG_QUERY, False)] * n_queries_to_execute)

    def test_03_no_overflow_with_limit(self):
        # execute query on each one of the threads
        n_queries_to_execute = self.conn.execute_command("GRAPH.CONFIG", "GET", "THREAD_COUNT")[1]

        # set query memory limit to 1GB
        limit = 1024*1024*1024
        self.conn.execute_command("GRAPH.CONFIG", "SET", "QUERY_MEM_CAPACITY", limit) 

        self.stress_server([(MEM_HOG_QUERY, False)] * n_queries_to_execute)

    def test_04_overflow_with_limit(self):
        # execute query on each one of the threads
        n_queries_to_execute = self.conn.execute_command("GRAPH.CONFIG", "GET", "THREAD_COUNT")[1]

        # set query memory limit to 1MB
        limit = 1024*1024
        self.conn.execute_command("GRAPH.CONFIG", "SET", "QUERY_MEM_CAPACITY", limit)

        self.stress_server([(MEM_HOG_QUERY, True)] * n_queries_to_execute)

    def test_05_test_mixed_queries(self):
        queries = []
        total_query_count = 100

        # Query the threadpool_size
        threadpool_size = self.conn.execute_command("GRAPH.CONFIG", "GET", "THREAD_COUNT")[1]

        # set query memory limit to 1MB
        limit = 1024*1024
        self.conn.execute_command("GRAPH.CONFIG", "SET", "QUERY_MEM_CAPACITY", limit)

        for i in range(total_query_count):
            should_fail = False
            q = MEM_THRIFTY_QUERY
            r = random.randint(0, 100)

            if r <= total_query_count * 0.1: # 10%
                q = MEM_HOG_QUERY
                should_fail = True

            queries.append((q, should_fail))

        self.stress_server(queries)
