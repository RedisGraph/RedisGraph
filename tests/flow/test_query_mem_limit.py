import random
import threading
from RLTest import Env
from redisgraph import Graph

# 1. test reading and setting query memory limit configuration

# 2. test overflowing the server when there's no limit,
#    expect no errors

# 3. test querying the server when there's a high memory limit and the queries
#    are all running under the current limit

# 4. test querying the server when there's a tight memory limit
#    expecting an out of memory error

n_failed = 0
error_encountered = False

g = None
GRAPH_NAME = "max_query_mem"
MEM_HOG_QUERY = """UNWIND range(0, 100000) AS x
                   WITH x
                   WHERE (x / 2) = 50
                   RETURN x, count(x)"""

def issue_query(conn, q):
    global error_encountered
    global n_failed

    try:
        g.query(q)
    except Exception as e:
        assert "Query's mem consumption exceeded capacity" in str(e)
        n_failed += 1
        error_encountered = True

class testQueryMemoryLimit():
    def __init__(self):
        global g
        self.env = Env(decodeResponses=True)
        self.conn = self.env.getConnection()
        g = Graph(GRAPH_NAME, self.conn)

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

    def stress_server(self, query):
        threads = []
        connections = []
        threadpool_size = self.conn.execute_command("GRAPH.CONFIG", "GET", "THREAD_COUNT")[1]

        # init connections
        for t in range(threadpool_size):
            connections.append(self.env.getConnection())

        # invoke queries
        while len(connections) > 0:
            con = connections.pop()
            t = threading.Thread(target=issue_query, args=(con, query))
            t.setDaemon(True)
            threads.append(t)
            t.start()

        # wait for threads to return
        while len(threads) > 0:
            t = threads.pop()
            t.join()

    def test_02_overflow_no_limit(self):
        global error_encountered
        error_encountered = False

        # set query memory limit as UNLIMITED
        limit = 0
        self.conn.execute_command("GRAPH.CONFIG", "SET", "QUERY_MEM_CAPACITY", limit) 

        self.stress_server(MEM_HOG_QUERY)

        # expecting no errors
        self.env.assertFalse(error_encountered)

    def test_03_no_overflow_with_limit(self):
        global error_encountered
        error_encountered = False

        # set query memory limit to 1GB
        limit = 1024*1024*1024
        self.conn.execute_command("GRAPH.CONFIG", "SET", "QUERY_MEM_CAPACITY", limit) 

        self.stress_server(MEM_HOG_QUERY)

        # expecting no errors
        self.env.assertFalse(error_encountered)

    def test_04_overflow_with_limit(self):
        global error_encountered
        error_encountered = False

        # set query memory limit to 1MB
        limit = 1024*1024
        self.conn.execute_command("GRAPH.CONFIG", "SET", "QUERY_MEM_CAPACITY", limit)

        self.stress_server(MEM_HOG_QUERY)

        # expecting out of memory error
        self.env.assertTrue(error_encountered)

    def test_05_test_mixed_queries(self):
        global n_failed
        global error_encountered
        n_failed = 0
        total_query_count = 100
        expected_error_count = 0
        error_encountered = False
        MEM_THRIFTY_QUERY = """UNWIND range(0, 10) AS x
                        WITH x
                        WHERE (x / 2) = 50
                        RETURN x, count(x)"""
        # Query the threadpool_size
        threadpool_size = self.conn.execute_command("GRAPH.CONFIG", "GET", "THREAD_COUNT")[1]

        # set query memory limit to 1MB
        limit = 1024*1024
        self.conn.execute_command("GRAPH.CONFIG", "SET", "QUERY_MEM_CAPACITY", limit)

        for i in range(total_query_count):
            r = random.randint(0, 100)
            q = MEM_THRIFTY_QUERY
            if r <= total_query_count * 0.1: # 10%
                q = MEM_HOG_QUERY
                expected_error_count += 1
            self.stress_server(q)

        # expecting out of memory error
        self.env.assertTrue(error_encountered)
        self.env.assertTrue(n_failed == expected_error_count*threadpool_size)

