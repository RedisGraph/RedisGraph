import random
import threading
from itertools import cycle
from RLTest import Env
from redisgraph import Graph

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

class testQueryMemoryLimit():
    def __init__(self):
        global g
        self.env = Env(decodeResponses=True)
        self.conn = self.env.getConnection()
        g = Graph(GRAPH_NAME, self.conn)

    def issue_query(self, conn, q, should_fail):
        try:
            g.query(q)
            self.env.assertFalse(should_fail)
        except Exception as e:
            assert "Query's mem consumption exceeded capacity" in str(e)
            self.env.assertTrue(should_fail)

    def stress_server(self, queries):
        threads = []
        connections = []
        threadpool_size = self.conn.execute_command("GRAPH.CONFIG", "GET", "THREAD_COUNT")[1]

        # init connections
        for t in range(threadpool_size):
            connections.append(self.env.getConnection())

        # init circular iterator
        connections_pool = cycle(connections)

        # invoke queries
        for q in queries:
            con = next(connections_pool)
            t = threading.Thread(target=self.issue_query, args=(con, q[0], q[1]))
            t.setDaemon(True)
            threads.append(t)
            t.start()

        # wait for threads to return
        while len(threads) > 0:
            t = threads.pop()
            t.join()

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
