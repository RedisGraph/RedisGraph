import multiprocessing as mp
from RLTest import Env
from redisgraph import Graph

# 1.test getting and setting config
# 2. test overflowing the server when there's a limit
#    expect to get error!
# 3. test overflowing the server when there's no limit
#    expect not to get any exceptions

con = None
error_encountered = False

GRAPH_NAME = "max_pending_queries"
# note: if test_03_overflow_with_limit is flaky 
# on machines with many cores on some runs the rate at which the queries been executed by the workers 
# might be greater than the insertion to the work queues. 
# This ends up were there is at most one job in the queue which failed
# test_03_overflow_with_limit. Increasing the execution time of the query should resolved this.
SLOW_QUERY = "UNWIND range (0, 100000) AS x WITH x WHERE (x / 2) = 50  RETURN x"

def issue_query(q):
    global con
    try:
        con.execute_command("GRAPH.QUERY", GRAPH_NAME, q)
        return False
    except Exception as e:
        assert "Max pending queries exceeded" in str(e)
        return True

def run_test_multiproc(env, n_procs, fn, args=tuple()):
    def init_process_local_connection():
        global con
        con = env.getConnection()
        return 1

    # on macOS the spawn start method is now the default one since python 3.8.
    # the spawn start methon fails when it stumbles upon objects that contain locks/fds
    # For this reason need to forcefully set the context to fork.
    # For more details see: https://bugs.python.org/issue33725
    # or the python docs: https://docs.python.org/3/library/multiprocessing.html#contexts-and-start-methods
    ctx = mp.get_context('fork')
    # crating connetion on pool init for increasing the fn calls rate 
    with ctx.Pool(n_procs, initializer=init_process_local_connection) as p:
        multiple_results = [p.apply_async(fn, args=args) for i in range(n_procs)]
        results = [res.get() for res in multiple_results]
        return results

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
        global error_encountered

        threads = []
        threadpool_size = self.conn.execute_command("GRAPH.CONFIG", "GET", "THREAD_COUNT")[1]
        thread_count = threadpool_size * 5

        results = run_test_multiproc(self.env, thread_count, issue_query, (SLOW_QUERY,))
        error_encountered = any(results)


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
