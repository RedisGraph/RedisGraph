import threading
from RLTest import Env
from redisgraph import Graph

# 1.test getting and setting config
# 2. test overflowing the server when there's a limit
#    expect to get error!
# 3. test overflowing the server when there's no limit
#    expect not to get any exceptions

g = None
error_encountered = False

GRAPH_NAME = "max_pending_queries"
FAST_QUERY = "RETURN 1"
SLOW_QUERY = "UNWIND range (0, 100000) AS x WITH x WHERE (x / 2) = 50  RETURN x"

def issue_query(conn, q):
    global error_encountered

    try:
        res = conn.execute_command("GRAPH.QUERY", GRAPH_NAME, q)
    except:
        error_encountered = True

class testPendingQueryLimit():
    def __init__(self):
        global g
        
        self.env = Env(decodeResponses=True)
        self.conn = self.env.getConnection()
        g = Graph(GRAPH_NAME, self.conn)
    
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

    def stress_server(self):
        threads = []
        connections = []
        thread_count = 60
        slow_query_count = 40
        fast_query_count = 20

        # init connections
        for i in range(thread_count):
            connections.append(self.env.getConnection())

        # invoke slow queries
        for i in range(slow_query_count):
            con = connections.pop()
            t = threading.Thread(target=issue_query, args=(con, SLOW_QUERY))
            t.setDaemon(True)
            threads.append(t)
            t.start()

        for i in range(fast_query_count):
            con = connections.pop()
            t = threading.Thread(target=issue_query, args=(con, FAST_QUERY))
            t.setDaemon(True)
            threads.append(t)
            t.start()

        # wait for threads to return
        for i in range(thread_count):
            t = threads[i]
            t.join()

    def test_02_overflow_no_limit(self):
        global error_encountered 
        error_encountered = False

        # no limit on number of pending queries
        limit = 4294967295
        self.conn.execute_command("GRAPH.CONFIG", "SET", "MAX_QUEUED_QUERIES", limit)

        self.stress_server()

        self.env.assertFalse(error_encountered)

    def test_03_overflow_with_limit(self):
        global error_encountered 
        error_encountered = False

        # limit number of pending queries
        limit = 10
        self.conn.execute_command("GRAPH.CONFIG", "SET", "MAX_QUEUED_QUERIES", limit)

        self.stress_server()

        self.env.assertTrue(error_encountered)

