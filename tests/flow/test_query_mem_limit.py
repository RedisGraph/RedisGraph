import threading
from RLTest import Env
from redisgraph import Graph

# 1. test getting and setting config
# 2. test overflowing the server when there's no limit
#    expect not to get any exceptions
# 2. test not overflowing the server when there's high limit
#    expect not to get any exceptions
# 4. test overflowing the server when there's a limit
#    expect to get error!

error_encountered = False

GRAPH_NAME = "max_query_mem"
MEM_HOG_QUERY = "UNWIND range (0, 100000) AS x WITH x WHERE (x / 2) = 50  RETURN x, count(x)"

def issue_query(conn, q):
    global error_encountered

    try:
        conn.execute_command("GRAPH.QUERY", GRAPH_NAME, q)
    except Exception as e:
        assert "Querie's mem consumption exceeded capacity" in str(e)
        error_encountered = True

class testPendingQueryLimit():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        self.conn = self.env.getConnection()

    def test_01_query_mem_limit_config(self):
        # read max queued queries config
        result = self.conn.execute_command("GRAPH.CONFIG", "GET", "QUERY_MEM_CAPACITY")
        query_mem_capacity = result[1]
        self.env.assertEquals(query_mem_capacity, 0)

        # update configuration, set capacity to some positive value
        self.conn.execute_command("GRAPH.CONFIG", "SET", "QUERY_MEM_CAPACITY", 1024*1024) #1MB

        # re-read configuration
        result = self.conn.execute_command("GRAPH.CONFIG", "GET", "QUERY_MEM_CAPACITY")
        query_mem_capacity = result[1]
        self.env.assertEquals(query_mem_capacity, 1024*1024)

    def stress_server(self):
        threads = []
        connections = []
        threadpool_size = self.conn.execute_command("GRAPH.CONFIG", "GET", "THREAD_COUNT")[1]

        # init connections
        connections.append(self.env.getConnection())

        # invoke queries
        con = connections.pop()
        t = threading.Thread(target=issue_query, args=(con, MEM_HOG_QUERY))
        t.setDaemon(True)
        threads.append(t)
        t.start()

        # wait for threads to return
        t = threads[0]
        t.join()

    def test_02_overflow_no_limit(self):
        global error_encountered
        error_encountered = False

        # no limit on querie's execution mem allocation
        limit = 0
        self.conn.execute_command("GRAPH.CONFIG", "SET", "QUERY_MEM_CAPACITY", limit) 

        self.stress_server()

        self.env.assertFalse(error_encountered)
        

    def test_03_no_overflow_with_limit(self):
        global error_encountered
        error_encountered = False

        # no limit on querie's execution mem allocation
        limit = 1024*1024*1024 #1GB
        self.conn.execute_command("GRAPH.CONFIG", "SET", "QUERY_MEM_CAPACITY", limit) 

        self.stress_server()

        self.env.assertFalse(error_encountered)

    def test_04_overflow_with_limit(self):
        global error_encountered
        error_encountered = False

        # limit number of pending queries
        limit = 1024*1024 #1MB
        self.conn.execute_command("GRAPH.CONFIG", "SET", "QUERY_MEM_CAPACITY", limit)

        self.stress_server()

        self.env.assertTrue(error_encountered)
