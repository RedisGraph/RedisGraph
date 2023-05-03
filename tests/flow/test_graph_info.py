import time
from common import *
from functools import partial
from pathos.pools import ProcessPool as Pool

graph = None
redis_con = None
GRAPH_X = '' # TODO: Replace '' this by '*'
GRAPH_1 = 'test_graph_info_1'
GRAPH_2 = 'test_graph_info_2'

QUERY_STAGE_WAITING = 0
QUERY_STAGE_EXECUTING = 1
QUERY_STAGE_REPORTING = 2
QUERY_STAGE_FINISHED = 3

INFO_QUERIES_COMMAND = 'GRAPH.INFO QUERIES %s'
INFO_QUERIES_CURRENT_COMMAND = 'GRAPH.INFO QUERIES %s CURRENT'
INFO_QUERIES_CURRENT_PREV_COMMAND_1 = 'GRAPH.INFO QUERIES %s CURRENT PREV 1'
INFO_QUERIES_CURRENT_PREV_COMMAND_5 = 'GRAPH.INFO QUERIES %s CURRENT PREV 5'

# Keys
GLOBAL_INFO_KEY_NAME = 'Global info'

def get_unix_timestamp_milliseconds():
    return (int)(time.time_ns() / 1000000)

def thread_run_query(query, **kwargs):
    """Runs a given query on a new thread (and client)"""
    
    if(kwargs):
        graph_name = kwargs['graph_name']
    else:
        graph_name = GRAPH_1

    env = Env(decodeResponses=True)
    conn = env.getConnection()
    graph = Graph(conn, graph_name)

    try:
        return graph.query(query)
    except ResponseError as e:
        return str(e)

def run_concurrently(queries, f, **kwargs):
    """Runs the given queries concurrently"""
    pool = Pool(nodes=len(queries))
    return pool.amap(partial(f, **kwargs), queries)

def run_separate_client(queries, **kwargs):
    """Runs the given queries on a separate client"""

    if isinstance(queries, list):
        return run_concurrently(queries, thread_run_query, **kwargs)
    else:
        return run_concurrently([queries], thread_run_query, **kwargs)

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
        graph = Graph(redis_con, GRAPH_1)

    def _assert_executed_query(self, query_info, expected_query_info, assert_receive_time=None):
        """Validate that query_info dictionary coincides with the expected result for a single query.
        Additionally, validates the duration numbers: Total = Wait + Execution + Report
        Optionally, checks for the receive time: all queries are checked to have
        been received after the passed "assert_receive_time"""

        if assert_receive_time is not None:
            self.env.assertGreaterEqual(query_info['Received at'], assert_receive_time, depth=1)

        # Assert Equals only for the keys that exists in expected_query_info.
        self.env.assertEquals(query_info, (query_info | expected_query_info), depth=1)

        # Validate duration data
        duration = query_info['Wait duration'] + query_info['Execution duration'] + query_info['Report duration']
        self.env.assertEquals(query_info['Total duration'], duration, depth=1)

    def execute_get_dict(self, query):
        """Executes a query and returns the result as a dictionary"""
        res = graph.execute_command(query)
        return list_to_dict(res)

    def _wait_till_query_info_changes(self, timeout = 2, graph_name = GRAPH_X):
        wait_step = 0.01
        waited_time = 0
        prev_info = self.execute_get_dict(INFO_QUERIES_CURRENT_COMMAND % graph_name)
        while True:
            info = self.execute_get_dict(INFO_QUERIES_CURRENT_COMMAND % graph_name)
            # Search for changes
            if (info[GLOBAL_INFO_KEY_NAME] != None and
                prev_info[GLOBAL_INFO_KEY_NAME] != info[GLOBAL_INFO_KEY_NAME]):
                return info
            time.sleep(wait_step)
            waited_time += wait_step
            prev_info = info
            if waited_time >= timeout:
                return None

    def _wait_till_queries_start_being_executed(self, query_count = 1, timeout = 2, graph_name = GRAPH_X):
        wait_step = 0.01
        waited_time = 0
        while True:
            info = self.execute_get_dict(INFO_QUERIES_CURRENT_COMMAND % graph_name)
            count = info[GLOBAL_INFO_KEY_NAME]['Total executing queries count']
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

    def test01_empty_info(self):
        """Tests that when no queries were executed, the 'CURRENT' info command
        returns an empty list"""

        info = self.conn.execute_command(INFO_QUERIES_CURRENT_COMMAND % GRAPH_X)
        res_dict = list_to_dict(info)
        self.env.assertEquals(res_dict['Queries'], [])

        info = self.conn.execute_command(INFO_QUERIES_CURRENT_PREV_COMMAND_1 % GRAPH_X)
        # TODO:
        # info = self.conn.execute_command("GRAPH.INFO QUERIES * PREV 1 CURRENT")
        res_dict = list_to_dict(info)
        self.env.assertEquals(res_dict['Queries'], [])

        info = self.conn.execute_command("GRAPH.INFO QUERIES CURRENT PREV 1")
        # TODO:
        # info = self.conn.execute_command("GRAPH.INFO QUERIES * CURRENT PREV 1")
        res_dict = list_to_dict(info)
        self.env.assertEquals(res_dict['Queries'], [])

    def test02_last_query(self):
        """Tests that the last query is returned by the `PREV 1` command"""

        # issue a query
        query = "MATCH (n) RETURN n"
        graph.query(query)
        info = self.execute_get_dict(INFO_QUERIES_CURRENT_PREV_COMMAND_1 % GRAPH_X)
        first_query = list_to_dict(info['Queries'][0])
        self.env.assertEquals(first_query['Query'], query)

        # issue another query
        query = "MATCH (n) RETURN n LIMIT 1"
        graph.query(query)
        info = self.execute_get_dict(INFO_QUERIES_CURRENT_PREV_COMMAND_1 % GRAPH_X)
        first_query = list_to_dict(info['Queries'][0])
        self.env.assertEquals(first_query['Query'], query)

    def test03_current_query(self):
        """Tests that the current query is returned by the `CURRENT` command"""

        query_issue_timestamp_ms = get_unix_timestamp_milliseconds()

        # issue a "heavy" query (long execution time)
        query = "UNWIND range(1, 10000000) as x CREATE (:Node {val: x})"
        async_res = run_separate_client(query, graph_name=GRAPH_1)

        # Wait until the concurrent query (UNWIND) starts execution: until
        # another redis client with the query above issues a query and it
        # starts being executed.
        self.env.assertTrue(self._wait_for_number_of_clients(2), depth=1)
        # Got two clients connected.

        # Wait until the query starts execution and get the statistics.
        info = self._wait_till_queries_start_being_executed(timeout=10)
        self.env.assertIsNotNone(info)

        # validate that the running query appears in the statistics
        expected_global_info = {
            'Current maximum query wait duration': 0,
            'Total waiting queries count': 0,
            'Total executing queries count': 1,
            'Total reporting queries count' : 0
        }
        self.env.assertEquals(info[GLOBAL_INFO_KEY_NAME], expected_global_info)

        # wait for the heavy query to finish
        async_res.wait()

        # assert the correctness of the results
        current_query = list_to_dict(info['Queries'][-1])
        expected_query_info = {
                'Stage' : QUERY_STAGE_EXECUTING,
                'Query' : query,
                'Graph name' : GRAPH_1,
                'Utilized cache' : 0
            }
        self._assert_executed_query(current_query, expected_query_info, query_issue_timestamp_ms)

    def test04_query_with_errors(self):
        """Test that queries with errors does not affect the global info"""

        query = "NULL QUERY"
        try:
            graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertContains("Invalid input", str(e))

        expected_global_info = {
            'Current maximum query wait duration': 0,
            'Total waiting queries count': 0,
            'Total executing queries count': 0,
            'Total reporting queries count' : 0
        }
        info = self.execute_get_dict(INFO_QUERIES_CURRENT_PREV_COMMAND_1 % GRAPH_X)
        self.env.assertEquals(info[GLOBAL_INFO_KEY_NAME], expected_global_info)

    def test05_invalid_info_commands(self):
        """Test commands with invalid arguments"""

        commands_with_errors = {
            "GRAPH.INFO" : "wrong number of arguments for 'graph.INFO' command",
            "GRAPH.INFO CURRENT" : "Unknown subcommand.",
            # TODO: Enable these tests after implementing the change to support GRAPH.INFO QUERIES <key>|*
            #       and fixing argument validation
            # "GRAPH.INFO QUERIES" : "wrong number of arguments for 'graph.INFO' command",
            # "GRAPH.INFO QUERIES * PREV 0" : """"PREV": Invalid value for the <count> parameter""",
            # "GRAPH.INFO QUERIES * PREV -1" : """"PREV": Invalid value for the <count> parameter""",
            # "GRAPH.INFO QUERIES * CURRENT CURRENT PREV PREV 1" : "???",
            # "GRAPH.INFO QUERIES * CURRENT PREV": """"PREV" : Invalid value for the <count> parameter""",
            # TODO: Enable these tests after implementing GRAPH.INFO GET
            # "GRAPH.INFO GET" : "wrong number of arguments for 'graph.INFO' command",
            # "GRAPH.INFO GET MEM" : "???",
            # "GRAPH.INFO GET * HELLO" : "???",
            # "GRAPH.INFO GET KEY HELLO" : "???",
            # "GRAPH.INFO GET * MEM MEM" : "???",
            # "GRAPH.INFO GET * MEM MEM" : "???",
            # TODO: Enable these tests after implementing GRAPH.INFO RESET
            # "GRAPH.INFO RESET" : "wrong number of arguments for 'graph.INFO' command",
            # "GRAPH.INFO RESET KEY KEY" : "wrong number of arguments for 'graph.INFO' command",
            # TODO: Enable these tests after implementing GRAPH.INFO CONFIG
            # "GRAPH.INFO CONFIG" : "wrong number of arguments for 'graph.INFO' command",
            # "GRAPH.INFO CONFIG PARAM PARAM" : "wrong number of arguments for 'graph.INFO' command",
        }
        for command, expected_err_msg in commands_with_errors.items():
            try:
                res = graph.execute_command(command)
                assert(False)
            except redis.exceptions.ResponseError as e:
                self.env.assertIn(expected_err_msg, str(e))

    def test06_query_info_single_query(self):
        """Test that valid query execution is reflected in query information"""

        # test twice to test the use of cache
        for cache in [0, 1]:
            # issue a query that returns results
            query = "UNWIND range(1,5000) AS i CREATE(n:N {id:i, v:rand()}) RETURN n.v"
            query_issue_timestamp_ms = get_unix_timestamp_milliseconds()
            graph.query(query)

            expected_query_info = {
                'Stage' : QUERY_STAGE_FINISHED,
                'Query' : query,
                'Graph name' : GRAPH_1,
                'Utilized cache' : cache
            }
            info = self.execute_get_dict(INFO_QUERIES_CURRENT_PREV_COMMAND_1 % GRAPH_X)
            first_query_dict = list_to_dict(info['Queries'][0])
            self._assert_executed_query(first_query_dict, expected_query_info, query_issue_timestamp_ms)

    def test07_query_info_parallel_execution_queries(self):
        """Test that global info matches with query details information"""

        # run queries in parallel
        query = """CYPHER end=100000 UNWIND (range(0, $end)) AS x WITH x AS x WHERE (x / 90000) = 1 RETURN x"""
        async_res0 = run_separate_client([query] * 2)
        async_res1 = run_separate_client([query] * 2)
        async_res2 = run_separate_client([query] * 1)

        # Wait for two clients connected
        self.env.assertTrue(self._wait_for_number_of_clients(2), depth=1)

        for i in range(1, 5):
            # Wait until some change is global info is detected
            info = self._wait_till_query_info_changes(timeout=3)
            info = self.execute_get_dict(INFO_QUERIES_CURRENT_PREV_COMMAND_5 % GRAPH_X)
            self.env.assertIsNotNone(info)

            nqueries = len(info['Queries'])
            waiting_queries = 0
            executing_queries = 0
            reporting_queries = 0
            for j in range(1, nqueries):
                current_query = list_to_dict(info['Queries'][j])
                if(current_query['Stage'] == QUERY_STAGE_WAITING):
                    waiting_queries += 1
                if(current_query['Stage'] == QUERY_STAGE_EXECUTING):
                    executing_queries += 1
                if(current_query['Stage'] == QUERY_STAGE_REPORTING):
                    reporting_queries += 1

            # Assert that global info matches with query info details
            self.env.assertEqual(info[GLOBAL_INFO_KEY_NAME]['Total waiting queries count'], waiting_queries)
            self.env.assertEqual(info[GLOBAL_INFO_KEY_NAME]['Total executing queries count'], executing_queries)
            self.env.assertEqual(info[GLOBAL_INFO_KEY_NAME]['Total reporting queries count'], reporting_queries)

        # Wait to finish the execution
        async_res0.wait();
        async_res1.wait();
        async_res2.wait();

        # Assert that all queries stopped
        info = self.execute_get_dict(INFO_QUERIES_COMMAND % GRAPH_X)
        self.env.assertIsNotNone(info, depth=1)
        expected_global_info = {
            'Total waiting queries count': 0,
            'Total executing queries count': 0,
            'Total reporting queries count' : 0
        }
        self.env.assertEquals(info[GLOBAL_INFO_KEY_NAME], (info[GLOBAL_INFO_KEY_NAME] | expected_global_info), depth=1)

    def test08_query_info_parallel_exec_multiple_graph(self):
        """Test that global info matches with query details information,
           execution of queries in two different graphs"""

        # run queries in parallel
        query = """CYPHER end=100 UNWIND (range(0, $end)) AS x WITH x AS x WHERE (x / 90000) = 1 RETURN x"""
        async_res0 = run_separate_client([query] * 2, graph_name=GRAPH_1)
        async_res1 = run_separate_client([query] * 2, graph_name=GRAPH_2)
        async_res2 = run_separate_client([query] * 1, graph_name=GRAPH_1)
        async_res2 = run_separate_client([query] * 1, graph_name=GRAPH_2)

        # Wait for two clients connected
        self.env.assertTrue(self._wait_for_number_of_clients(2), depth=1)

        for i in range(1, 5):
            # Wait until some change is global info is detected
            info = self._wait_till_query_info_changes(timeout=3)
            info = self.execute_get_dict('GRAPH.INFO QUERIES * PREV 6')
            self.env.assertIsNotNone(info)

            nqueries = len(info['Queries'])
            waiting_queries = 0
            executing_queries = 0
            reporting_queries = 0
            received_time = 0
            for j in range(1, nqueries):
                current_query = list_to_dict(info['Queries'][j])
                if(current_query['Stage'] == QUERY_STAGE_WAITING):
                    waiting_queries += 1
                if(current_query['Stage'] == QUERY_STAGE_EXECUTING):
                    executing_queries += 1
                if(current_query['Stage'] == QUERY_STAGE_REPORTING):
                    reporting_queries += 1
                # queries must be ordered by increasing “received at” time
                self.env.assertGreaterEqual(current_query['Received at'], received_time)
                received_time = current_query['Received at']

            # Assert that global info matches with query info details
            self.env.assertEqual(info[GLOBAL_INFO_KEY_NAME]['Total waiting queries count'], waiting_queries)
            self.env.assertEqual(info[GLOBAL_INFO_KEY_NAME]['Total executing queries count'], executing_queries)
            self.env.assertEqual(info[GLOBAL_INFO_KEY_NAME]['Total reporting queries count'], reporting_queries)

        # Wait to finish the execution
        async_res0.wait();
        async_res1.wait();
        async_res2.wait();

        # Assert that all queries stopped
        info = self.execute_get_dict(INFO_QUERIES_COMMAND % GRAPH_X)
        self.env.assertIsNotNone(info, depth=1)
        expected_global_info = {
            'Total waiting queries count': 0,
            'Total executing queries count': 0,
            'Total reporting queries count' : 0
        }
        self.env.assertEquals(info[GLOBAL_INFO_KEY_NAME], (info[GLOBAL_INFO_KEY_NAME] | expected_global_info), depth=1)
