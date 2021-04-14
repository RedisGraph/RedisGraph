import threading
from RLTest import Env
from redisgraph import Graph

# 1.test getting and setting config
# 2. test overflowing the server when there's a limit
#    expect to get error!
# 3. test overflowing the server when there's no limit
#    expect not to get any exceptions

error_encountered = False

GRAPH_NAME = "max_pending_queries"
SLOW_QUERY = "UNWIND range (0, 100000) AS x WITH x WHERE (x / 2) = 50  RETURN x"

def issue_query(conn, q):
    global error_encountered

    try:
        conn.execute_command("GRAPH.QUERY", GRAPH_NAME, q)
    except Exception as e:
        assert "Max pending queries exceeded" in str(e)
        error_encountered = True

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

    def stress_server(self):
        threads = []
        connections = []
        thread_count = 60

        # init connections
        for i in range(thread_count):
            connections.append(self.env.getConnection())

        # invoke queries
        for i in range(thread_count):
            con = connections.pop()
            t = threading.Thread(target=issue_query, args=(con, SLOW_QUERY))
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
        limit = 1
        self.conn.execute_command("GRAPH.CONFIG", "SET", "MAX_QUEUED_QUERIES", limit)

        self.stress_server()

        self.env.assertTrue(error_encountered)
