import time
from common import *
from pathos.pools import ProcessPool as Pool

graph = None
redis_con = None
GRAPH_ID = 'test_graph_info'

QUERY_STAGE_WAITING = 0
QUERY_STAGE_EXECUTING = 1
QUERY_STAGE_REPORTING = 2
QUERY_STAGE_FINISHED = 3

INFO_QUERIES_COMMAND = 'GRAPH.INFO QUERIES'
INFO_QUERIES_CURRENT_COMMAND = 'GRAPH.INFO QUERIES CURRENT'
INFO_QUERIES_CURRENT_PREV_COMMAND_1 = 'GRAPH.INFO QUERIES CURRENT PREV 1'
INFO_QUERIES_CURRENT_PREV_COMMAND_5 = 'GRAPH.INFO QUERIES CURRENT PREV 5'

def get_unix_timestamp_milliseconds():
    return (int)(time.time_ns() / 1000000)

def thread_run_query(query):
    """Runs a given query on a new thread (and client)"""

    env = Env(decodeResponses=True)
    conn = env.getConnection()
    graph = Graph(conn, GRAPH_ID)

    try:
        return graph.query(query)
    except ResponseError as e:
        return str(e)

def run_concurrently(queries, f):
    """Runs the given queries concurrently"""
    pool = Pool(nodes=len(queries))
    return pool.amap(f, queries)

def run_separate_client(queries):
    """Runs the given queries on a separate client"""

    if isinstance(queries, list):
        return run_concurrently(queries, thread_run_query)
    else:
        return run_concurrently([queries], thread_run_query)

def list_to_dict(lst):
    """Recursively converts a list to a dictionary.
    This is useful when we convert the response of the `execute_command` method
    to a dictionary, since the response is a list of alternating keys and
    values."""

    d = {}
    for i in range(0, len(lst), 2):
        k = lst[i]
        if isinstance(k, list):
            return k
        if i + 1 >= len(lst):
            d[k] = None
            break
        v = lst[i + 1]
        if isinstance(v, list) and len(v) > 0 and isinstance(v[0], str):
            v = list_to_dict(v)
        d[k] = v
    return d

class testGraphInfo(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)
        self.conn = self.env.getConnection()
        global graph
        global redis_con
        redis_con = self.env.getConnection()
        graph = Graph(redis_con, GRAPH_ID)

    def get_total_executing_queries_from_info_cmd_result(self, info):
        """Returns the total number of executing queries from the result of an
        INFO QUERIES command"""
        return info[1][5]

    def _wait_till_queries_start_being_executed(self, query_count=1, timeout=2):
        wait_step = 0.01
        waited_time = 0
        while True:
            info = self.conn.execute_command(INFO_QUERIES_CURRENT_COMMAND)
            count = self.get_total_executing_queries_from_info_cmd_result(info)
            if count >= query_count:
                # Found the query being executed.
                return info
            time.sleep(wait_step)
            waited_time += wait_step
            if waited_time >= timeout:
                return None

    def _wait_for_number_of_clients(self, clients_count, timeout=2):
        wait_step = 0.1
        waited_time = 0
        lookup_string = 'connected_clients:%d' % clients_count
        while lookup_string not in self.conn.execute_command('INFO CLIENTS'):
            time.sleep(wait_step)
            waited_time += wait_step
            if waited_time >= timeout:
                return False
        return True

    def execute_get_dict(self, query):
        """Executes a query and returns the result as a dictionary"""
        res = graph.execute_command(query)
        return list_to_dict(res)

    def assertEquals(self, a, b):
        self.env.assertEquals(a, b)

    def test01_empty_info(self):
        """Tests that when no queries were executed, the 'CURRENT' info command
        returns an empty list"""

        info = self.conn.execute_command(INFO_QUERIES_CURRENT_COMMAND)
        res_dict = list_to_dict(info)
        self.env.assertEquals(res_dict['Queries'], [])

        info = self.conn.execute_command(INFO_QUERIES_CURRENT_PREV_COMMAND_1)
        res_dict = list_to_dict(info)
        self.env.assertEquals(res_dict['Queries'], [])

    def test02_last_query(self):
        """Tests that the last query is returned by the `PREV 1` command"""

        # issue a query
        query = "MATCH (n) RETURN n"
        graph.query(query)
        info = self.execute_get_dict(INFO_QUERIES_CURRENT_PREV_COMMAND_1)
        first_query = list_to_dict(info['Queries'][0])
        self.env.assertEquals(first_query['Query'], query)

        # issue another query
        query = "MATCH (n) RETURN n LIMIT 1"
        graph.query(query)
        info = self.execute_get_dict(INFO_QUERIES_CURRENT_PREV_COMMAND_1)
        first_query = list_to_dict(info['Queries'][0])
        self.assertEquals(first_query['Query'], query)

    def test03_current_query(self):
        """Tests that the current query is returned by the `CURRENT` command"""

        query_issue_timestamp_ms = get_unix_timestamp_milliseconds()

        # issue a "heavy" query (long execution time)
        query = "UNWIND range(1, 10000000) as x CREATE (:Node {val: x})"
        async_res = run_separate_client(query)

        # Wait until the concurrent query (UNWIND) starts execution: until
        # another redis client with the query above issues a query and it
        # starts being executed.
        self.env.assertTrue(self._wait_for_number_of_clients(2), depth=1)
        # Got two clients connected.

        # Wait until the query starts execution and get the statistics.
        info = self._wait_till_queries_start_being_executed(timeout=10)
        self.env.assertIsNotNone(info)

        # wait for the heavy query to finish
        async_res.wait()

        # assert the correctness of the results
        current_query = list_to_dict(list_to_dict(info)['Queries'][-1])
        self.env.assertGreaterEqual(current_query['Received at'],
            query_issue_timestamp_ms, depth=1)
        self.env.assertEqual(current_query['Stage'], QUERY_STAGE_EXECUTING,
            depth=1)
        self.env.assertEqual(current_query['Graph name'], GRAPH_ID, depth=1)
        self.env.assertEqual(current_query['Query'], query, depth=1)








# TODO: Add tests for the following fields (values are examples):
    # "Global info"
        # "Current maximum query wait duration"
        # (integer) 0

        # "Total waiting queries count"
        # (integer) 0

        # "Total executing queries count"
        # (integer) 0

        # "Total reporting queries count"
        # (integer) 0


    # "Queries"
        # "Received at"
        # (integer) 1682593878897

        # "Stage"
        # (integer) 3

        # "Graph name"
        # "g"

        # "Query"
        # "create (:N)"

        # "Total duration"
        # (integer) 0

        # "Wait duration"
        # (integer) 0

        # "Execution duration"
        # (integer) 0

        # "Report duration"
        # (integer) 0

        # "Utilized cache"
        # (integer) 0
