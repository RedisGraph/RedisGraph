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

error_encountered = False

g = None
GRAPH_NAME = "max_query_mem"
MEM_HOG_QUERY = """UNWIND range(0, 100000) AS x
                   WITH x
                   WHERE (x / 2) = 50
                   RETURN x, count(x)"""

def issue_query(conn, q):
    global error_encountered

    try:
        g.query(q)
    except Exception as e:
        assert "Querie's mem consumption exceeded capacity" in str(e)
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

    def stress_server(self):
        threads = []
        connections = []
        threadpool_size = self.conn.execute_command("GRAPH.CONFIG", "GET", "THREAD_COUNT")[1]

        # init connections
        for t in range(threadpool_size):
            connections.append(self.env.getConnection())

        # invoke queries
        while len(connections) > 0:
            con = connections.pop()
            t = threading.Thread(target=issue_query, args=(con, MEM_HOG_QUERY))
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

        self.stress_server()

        # expecting no errors
        self.env.assertFalse(error_encountered)

    def test_03_no_overflow_with_limit(self):
        global error_encountered
        error_encountered = False

        # set query memory limit to 1GB
        limit = 1024*1024*1024
        self.conn.execute_command("GRAPH.CONFIG", "SET", "QUERY_MEM_CAPACITY", limit) 

        self.stress_server()

        # expecting no errors
        self.env.assertFalse(error_encountered)

    def test_04_overflow_with_limit(self):
        global error_encountered
        error_encountered = False

        # set query memory limit to 1MB
        limit = 1024*1024
        self.conn.execute_command("GRAPH.CONFIG", "SET", "QUERY_MEM_CAPACITY", limit)

        self.stress_server()

        # expecting out of memory error
        self.env.assertTrue(error_encountered)

