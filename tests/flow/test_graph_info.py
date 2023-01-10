# GRAPH.INFO being useful only concurrently, requires concurrent tests.
import asyncio
from common import *
from pathos.pools import ProcessPool as Pool
from pathos.helpers import mp as pathos_multiprocess

from common import *
import time
from enum import Enum

GRAPH_ID = "GRAPH_INFO_TEST"

# Commands
INFO_QUERIES_CURRENT_COMMAND = 'GRAPH.INFO QUERIES CURRENT'
INFO_QUERIES_PREV_COMMAND = 'GRAPH.INFO QUERIES PREV 100 --compact'
INFO_QUERIES_CURRENT_PREV_COMMAND = 'GRAPH.INFO QUERIES CURRENT PREV 100'
INFO_GET_GENERIC_COMMAND_TEMPLATE = 'GRAPH.INFO GET %s'
INFO_GET_STAT_COMMAND_TEMPLATE = 'GRAPH.INFO GET %s STAT'
INFO_RESET_ALL_COMMAND = 'GRAPH.INFO RESET *'

# Error messages
COULDNOT_FIND_GRAPH_ERROR_MESSAGE = "Couldn't find the specified graph"
TIMEOUT_ERROR_MESSAGE = "Query timed out"
RUNTIME_FAILURE_MESSAGE = "Division by zero"

# Keys
CURRENT_MAXIMUM_WAIT_DURATION_KEY_NAME = 'Current maximum query wait duration'
GLOBAL_INFO_KEY_NAME = 'Global info'
QUERY_STAGE_EXECUTING = 1
QUERY_STAGE_FINISHED = 3

# Other
LONG_CALCULATION_QUERY = """UNWIND (range(0, 10000000)) AS x WITH x AS x WHERE (x / 90000) = 1 RETURN x"""
PERCENTILE_COUNTS_COUNT = 6


class ReadOnlyQueryKind(Enum):
    RO_QUERY = 0
    QUERY = 1
    EXPLAIN = 2
    PROFILE = 3

ALL_READONLY_KINDS = [
    ReadOnlyQueryKind.RO_QUERY,
    ReadOnlyQueryKind.QUERY,
    ReadOnlyQueryKind.EXPLAIN,
    ReadOnlyQueryKind.PROFILE
]


class WriteQueryKind(Enum):
    QUERY = 0
    PROFILE = 1

ALL_WRITE_KINDS = [
    WriteQueryKind.QUERY,
    WriteQueryKind.PROFILE
]

class QueryFailureSimulationKind(Enum):
    TIMEOUT = 0
    FAIL_RUNTIME = 1
    FAIL_EARLY = 2


def thread_execute_command(cmd_and_args, _args):
    env = Env(decodeResponses=True)
    conn = env.getConnection()
    graph = Graph(conn, GRAPH_ID)

    try:
        (cmd, *args) = cmd_and_args
        return graph.execute_command(cmd, *args)
    except ResponseError as e:
        return str(e)

def thread_run_query(query):
    env = Env(decodeResponses=True)
    conn = env.getConnection()
    graph = Graph(conn, GRAPH_ID)

    try:
        return graph.query(query)
    except ResponseError as e:
        return str(e)

def run_concurrently(queries, f):
    pool = Pool(nodes=len(queries))
    return pool.apipe(f, queries)

def list_to_dict(lst):
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

# This won't work in the compact mode as the output is different.
def get_total_executing_queries_from_info_cmd_result(info):
    # [1][5] = 'Global info' => 'Total executing queries count'
    return info[1][5]

def get_unix_timestamp_milliseconds():
    import time

    return (int)(time.time_ns() / 1000000)


class _testGraphInfoFlowBase(FlowTestsBase):
    def _delete_graph(self):
        graph = Graph(self.conn, GRAPH_ID)
        try:
            graph.delete()
        except redis.exceptions.ResponseError:
            pass

    def _create_graph_filled(self):
        graph = Graph(self.conn, GRAPH_ID)

        for i in range(0, 2):
            graph.add_node(Node(label='Person', properties={'age': i}))

        graph.commit()
        return i + 1

    def _create_graph_empty(self):
        self.conn.execute_command('GRAPH.QUERY', GRAPH_ID, 'RETURN 1')

    def _recreate_graph_empty(self):
        self._delete_graph()
        self._create_graph_empty()

    def _recreate_graph_with_node(self):
        self._delete_graph()
        return self._create_graph_filled()

    def __init__(self, env):
        self.env = env
        # skip test if we're running under Valgrind
        if self.env.envRunner.debugger is not None:
            self.env.skip() # valgrind is not working correctly with multi processing
        self.conn = self.env.getConnection()
        self._recreate_graph_with_node()

    # Validate the GRAPH.INFO result: should contain the queries specified,
    # and exactly thath number of queries being executed.
    # Optionally, checks for the receive time: all queries are checked to have
    # been received after the passed "assert_receive_time".
    def _assert_executing_queries(self, info, queries=[], assert_receive_time=None):
        assert_timestamp = get_unix_timestamp_milliseconds()
        # if the info object is passed directly as a result, convert it to
        # a dictionary for easier and consistent assertions.
        if not isinstance(info, dict):
            info = list_to_dict(info)
        # No other queries has finished yet.
        # No queries are waiting to be executed as the only query we
        # launched must have already started being executed.
        self.env.assertEquals(info[GLOBAL_INFO_KEY_NAME]['Total waiting queries count'], 0, depth=1)
        # The current query is being executed.
        executing_queries_count = len(queries)
        self.env.assertEquals(info[GLOBAL_INFO_KEY_NAME]['Total executing queries count'], executing_queries_count, depth=1)
        # As there are no other queries, no query is reporting.
        self.env.assertEquals(info[GLOBAL_INFO_KEY_NAME]['Total reporting queries count'], 0, depth=1)
        max_wait_time = 0
        # Check that the only query being currently executed is ours.
        for i in range(0, executing_queries_count):
            executing_query = list_to_dict(info['Queries'][i])
            if assert_receive_time is not None:
                self.env.assertGreaterEqual(executing_query['Received at'], assert_receive_time, depth=1)
            self.env.assertLessEqual(executing_query['Received at'], assert_timestamp, depth=1)
            self.env.assertEquals(executing_query['Graph name'], GRAPH_ID, depth=1)
            self.env.assertEquals(executing_query['Stage'], 1, depth=1)
            self.env.assertEquals(executing_query['Query'], queries[i], depth=1)
            self.env.assertGreaterEqual(executing_query['Total duration'], 0, depth=1)
            wait_time = executing_query['Wait duration']
            max_wait_time = max(max_wait_time, wait_time)
            self.env.assertGreaterEqual(wait_time, 0, depth=1)
            self.env.assertGreaterEqual(executing_query['Execution duration'], 0, depth=1)
            self.env.assertEqual(executing_query['Report duration'], 0, depth=1)
        # It has already accounted for the graph creation query, which might
        # have taken longer than a millisecond.
        self.env.assertGreaterEqual(info[GLOBAL_INFO_KEY_NAME][CURRENT_MAXIMUM_WAIT_DURATION_KEY_NAME], max_wait_time, depth=1)

    def _assert_one_executing_query(self, info, query, assert_receive_time=None):
        queries = []
        if query is not None:
            queries.append(query)
        self._assert_executing_queries(info, queries, assert_receive_time)

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

    def _wait_till_queries_start_being_executed(self, query_count=1, timeout=2, prev=False):
        wait_step = 0.1
        waited_time = 0
        while True:
            cmd = INFO_QUERIES_CURRENT_COMMAND
            if prev == True:
                cmd = INFO_QUERIES_CURRENT_PREV_COMMAND
            info = self.conn.execute_command(cmd)
            count = get_total_executing_queries_from_info_cmd_result(info)
            if count >= query_count:
                # Found the query being executed.
                return info
            time.sleep(wait_step)
            waited_time += wait_step
            if waited_time >= timeout:
                return None

    def _assert_info_get_result(self, result,
        nodes=0, relationships=0,
        node_labels=0, relationship_types=0,
        node_indices=0, relationship_indices=0,
        node_property_names=0, edge_property_names=0,
        stat=None):
        info = list_to_dict(result)
        self.env.assertEqual(info['Number of nodes'], nodes)
        self.env.assertEqual(info['Number of relationships'], relationships)
        self.env.assertEqual(info['Number of node labels'], node_labels)
        self.env.assertEqual(info['Number of relationship types'], relationship_types)
        self.env.assertEqual(info['Number of node indices'], node_indices)
        self.env.assertEqual(info['Number of relationship indices'], relationship_indices)
        self.env.assertEqual(info['Total number of node properties'], node_property_names)
        self.env.assertEqual(info['Total number of edge properties'], edge_property_names)

    def _assert_info_get_counts(self, result,
    total_query_count=0, successful_readonly=0, successful_write=0,
    failed_readonly=0, failed_write=0, timedout_readonly=0, timedout_write=0):
        info = list_to_dict(result)
        self.env.assertEqual(info['Total number of queries'], total_query_count)
        self.env.assertEqual(info['Successful read-only queries'], successful_readonly)
        self.env.assertEqual(info['Successful write queries'], successful_write)
        self.env.assertEqual(info['Failed read-only queries'], failed_readonly)
        self.env.assertEqual(info['Failed write queries'], failed_write)
        self.env.assertEqual(info['Timed out read-only queries'], timedout_readonly)
        self.env.assertEqual(info['Timed out write queries'], timedout_write)

    def _assert_info_get_stat(self, result,
    total_durations=[], wait_durations=[],
    execution_durations=[], report_durations=[]):
        info = list_to_dict(result)
        self.env.assertEqual(info['Query total durations'], total_durations)
        self.env.assertEqual(info['Query wait durations'], wait_durations)
        self.env.assertEqual(info['Query execution durations'], execution_durations)
        self.env.assertEqual(info['Query report durations'], report_durations)


    # Runs a read-only query. A read-only query isn't just based on whether
    # it is run via GRAPH.RO_QUERY but it can also be an ordinary GRAPH.QUERY
    # that just doens't change anything. In the code we analyse the AST of the
    # received query and so determine whether it is a write or read-only query.
    #
    # That said, we should check that both of the cases lead to the same result.
    #
    # This uses the execute_command instead of graph methods as the graph
    # methods silently perform other queries which break the statistics.
    #
    # Additionally, this function may simulate a problem for a query.
    # This is useful for counting the number of failed queries.
    def _run_readonly_query(self, kind: ReadOnlyQueryKind, problem_kind=None):
        timeout = 0
        query = 'MATCH (pppp) RETURN pppp'
        if kind == ReadOnlyQueryKind.EXPLAIN:
            query = 'CREATE (p:Person) RETURN p'

        if problem_kind == QueryFailureSimulationKind.TIMEOUT:
            assert kind != ReadOnlyQueryKind.EXPLAIN, \
                'the EXPLAIN queries never time out'
            query = LONG_CALCULATION_QUERY
            timeout = 1
        elif problem_kind == QueryFailureSimulationKind.FAIL_RUNTIME:
            assert kind != ReadOnlyQueryKind.EXPLAIN, \
                'the EXPLAIN queries never fail at runtime'
            # Such queries will be successfully parsed and will require a
            # traversal during the execution. The parser can't know here whether
            # it may fail or not, so it successfully builds an execution plan
            # for the query and begins the execution.
            query = 'MATCH (p:Person), (p2:Person) RETURN p2.age / p.age'
        elif problem_kind == QueryFailureSimulationKind.FAIL_EARLY:
            # Parsing error is an example "fail early" thing.
            # Another example is when the parser immediately sees the
            # division by zero.
            query = 'RETURN 1/0'
        elif problem_kind is not None:
            assert False, \
                f"Unknown failure simulation kind: {problem_kind}"

        if kind == ReadOnlyQueryKind.RO_QUERY:
            return self.conn.execute_command('GRAPH.RO_QUERY', GRAPH_ID, query, 'TIMEOUT', timeout)
        elif kind == ReadOnlyQueryKind.QUERY:
            return self.conn.execute_command('GRAPH.QUERY', GRAPH_ID, query, 'TIMEOUT', timeout)
        elif kind == ReadOnlyQueryKind.EXPLAIN:
            graph = Graph(self.conn, GRAPH_ID)
            return graph.explain(query)
        elif kind == ReadOnlyQueryKind.PROFILE:
            return self.conn.execute_command('GRAPH.PROFILE', GRAPH_ID, query, 'TIMEOUT', timeout)
        else:
            assert False, \
                f"Unknown read only kind: {kind}"

    def _run_write_query(self,
    kind: WriteQueryKind, problem_kind=None):
        query = 'CREATE (m:Miracle) RETURN m'
        timeout = 0

        if problem_kind == QueryFailureSimulationKind.TIMEOUT:
            query = 'UNWIND (range(0, 1000000)) AS x WITH x CREATE(b:Book { id: x })'
            timeout = 1
        elif problem_kind == QueryFailureSimulationKind.FAIL_RUNTIME:
            query = 'CREATE (t:T { n: 0 }), (t2:T { n: 20 }) RETURN t2.n / t.n'
        elif problem_kind == QueryFailureSimulationKind.FAIL_EARLY:
            query = 'RETURN 1/0'
        elif problem_kind is not None:
            assert False, \
                f"Unknown failure simulation kind: {problem_kind}"

        if kind == WriteQueryKind.QUERY:
            return self.conn.execute_command('GRAPH.QUERY', GRAPH_ID, query, 'TIMEOUT', timeout)
        elif kind == WriteQueryKind.PROFILE:
            return self.conn.execute_command('GRAPH.PROFILE', GRAPH_ID, query, 'TIMEOUT', timeout)
        else:
            assert False, \
                f"Unknown write kind: {kind}"


class testGraphInfoFlow(_testGraphInfoFlowBase):
    def __init__(self):
        _testGraphInfoFlowBase.__init__(self, Env(decodeResponses=True))

    def test01_empty_graph_info(self):
        info = self.conn.execute_command(INFO_QUERIES_CURRENT_COMMAND)
        self._assert_one_executing_query(info, None)

    def test02_long_query_is_recorded_as_being_executed(self):
        query_issue_timestamp_ms = get_unix_timestamp_milliseconds()
        query = """UNWIND (range(0, 10000000)) AS x WITH x AS x WHERE (x / 90000) = 1 RETURN x"""
        waiter = run_concurrently((query), thread_run_query)

        # Wait until the concurrent query (UNWIND) starts execution: until
        # another redis client with the query above issues a query and it
        # starts being executed.
        self.env.assertTrue(self._wait_for_number_of_clients(2), depth=1)
        # Got two clients connected.

        # Wait until the query starts execution and get the statistics.
        info = self._wait_till_queries_start_being_executed()
        self.env.assertIsNotNone(info, depth=1)

        waiter.wait()
        results = waiter.get().result_set
        # Validate the GRAPH.QUERY result.
        self.env.assertEquals(results[0][0], 90000)
        self._assert_one_executing_query(info, query, assert_receive_time=query_issue_timestamp_ms)

    def test03_graph_info_reset_all(self):
        info = self.conn.execute_command(INFO_QUERIES_CURRENT_COMMAND)
        self._assert_one_executing_query(info, None)

        query = """UNWIND (range(0, 1000000)) AS x WITH x AS x WHERE (x / 90000) = 1 RETURN x"""
        graph = Graph(self.conn, GRAPH_ID)
        results = graph.query(query)
        self.env.assertEquals(results.result_set[0][0], 90000, depth=1)

        info = list_to_dict(self.conn.execute_command(INFO_QUERIES_CURRENT_COMMAND))
        self.env.assertGreaterEqual(info[GLOBAL_INFO_KEY_NAME][CURRENT_MAXIMUM_WAIT_DURATION_KEY_NAME], 0, depth=1)

        reset = self.conn.execute_command(INFO_RESET_ALL_COMMAND)
        self.env.assertTrue(reset, depth=1)

        info = list_to_dict(self.conn.execute_command(INFO_QUERIES_CURRENT_COMMAND))
        self.env.assertEqual(info[GLOBAL_INFO_KEY_NAME][CURRENT_MAXIMUM_WAIT_DURATION_KEY_NAME], 0, depth=1)

    def test04_graph_info_get_current_graph_generic(self):
        info = self.conn.execute_command(INFO_GET_GENERIC_COMMAND_TEMPLATE % GRAPH_ID)
        self._assert_info_get_result(info, nodes=2, node_labels=1, node_property_names=2)
        query = """MATCH (p:Person) CREATE (p2:Person { Name: 'Victor', Country: 'The Netherlands' })-[e:knows { Since_Year: '1970'}]->(p)"""
        graph = Graph(self.conn, GRAPH_ID)
        result = graph.query(query)
        self.env.assertEquals(result.nodes_created, 2, depth=1)
        self.env.assertEquals(result.relationships_created, 2, depth=1)
        self.env.assertEquals(result.properties_set, 6, depth=1)

        info = self.conn.execute_command(INFO_GET_GENERIC_COMMAND_TEMPLATE % GRAPH_ID)
        self._assert_info_get_result(info, nodes=4, node_labels=1, relationships=2, relationship_types=1, node_property_names=6, edge_property_names=2)

    def test04_graph_info_get_current_graph_stat(self):
        self._recreate_graph_empty()
        info = self.conn.execute_command(INFO_GET_STAT_COMMAND_TEMPLATE % GRAPH_ID)
        info = list_to_dict(info)

        '''
        When there have been no queries for this graph, the counters should
        be all set to zero.
        '''
        total_durations = [0] * PERCENTILE_COUNTS_COUNT
        wait_durations = [0] * PERCENTILE_COUNTS_COUNT
        execution_durations = [0] * PERCENTILE_COUNTS_COUNT
        report_durations = [0] * PERCENTILE_COUNTS_COUNT

        self.env.assertEqual(info['Query total durations'], total_durations, depth=1)
        self.env.assertEqual(info['Query wait durations'], wait_durations, depth=1)
        self.env.assertEqual(info['Query execution durations'], execution_durations, depth=1)
        self.env.assertEqual(info['Query report durations'], report_durations, depth=1)

        '''When there a query has been performed, the counters should reflect
        the durations respectively.'''
        # Execute a query.
        query = """UNWIND (range(0, 1000000)) AS x WITH x AS x WHERE (x / 90000) = 1 RETURN x"""
        graph = Graph(self.conn, GRAPH_ID)
        results = graph.query(query)
        execution_time = results.statistics['internal execution time']

        # Get new statistics.
        info = self.conn.execute_command(INFO_GET_STAT_COMMAND_TEMPLATE % GRAPH_ID)
        info = list_to_dict(info)

        # The error margin in milliseconds allowed.
        error_margin = 10

        # Now that we can't know the internal wait, execution and report time
        # from the outside, we just check that those are greater than zero and
        # less than the total execution time.
        got_total_durations = info['Query total durations']
        self.env.assertEqual(got_total_durations[0:2], [0, 0], depth=1)
        # The values should be in the range of execution time +- error margin.
        self.env.assertGreaterEqual(got_total_durations[2:6], [execution_time - error_margin] * 4, depth=1)
        self.env.assertLessEqual(got_total_durations[2:6], [execution_time + error_margin] * 4, depth=1)

        got_wait_durations = info['Query wait durations']
        self.env.assertEqual(got_wait_durations[0:2], [0, 0], depth=1)
        # The values should be in the range of [0;execution time]
        self.env.assertGreaterEqual(got_wait_durations[2:6], [0] * 4, depth=1)
        self.env.assertLessEqual(got_wait_durations[2:6], [execution_time] * 4, depth=1)

        got_execution_durations = info['Query wait durations']
        self.env.assertEqual(got_execution_durations[0:2], [0, 0], depth=1)
        # The values should be in the range of [0;execution time]
        self.env.assertGreaterEqual(got_execution_durations[2:6], [0] * 4, depth=1)
        self.env.assertLessEqual(got_execution_durations[2:6], [execution_time] * 4, depth=1)

        got_report_durations = info['Query report durations']
        self.env.assertEqual(got_report_durations[0:2], [0, 0], depth=1)
        # The values should be in the range of [0;execution time]
        self.env.assertGreaterEqual(got_report_durations[2:6], [0] * 4, depth=1)
        self.env.assertLessEqual(got_report_durations[2:6], [execution_time] * 4, depth=1)

    def test05_graph_info_queries_prev(self):
        query = """CYPHER end=100 RETURN reduce(sum = 0, n IN range(1, $end) | sum ^ n)"""
        graph = Graph(self.conn, GRAPH_ID)
        query_issue_timestamp_ms = get_unix_timestamp_milliseconds()

        results = graph.query(query)
        self.env.assertEquals(results.result_set[0][0], 0, depth=1)

        results = self.conn.execute_command(INFO_QUERIES_PREV_COMMAND)
        last_query = results[1][-1]
        self.env.assertGreaterEqual(last_query[0], query_issue_timestamp_ms, depth=1)
        self.env.assertEqual(last_query[1], QUERY_STAGE_FINISHED, depth=1)
        self.env.assertEqual(last_query[2], GRAPH_ID, depth=1)
        self.env.assertEqual(last_query[3], query, depth=1)

    def test06_graph_info_queries_prev_and_current(self):
        prev_query_string = """CYPHER end=100 RETURN reduce(sum = 0, n IN range(1, $end) | sum ^ n)"""
        graph = Graph(self.conn, GRAPH_ID)

        results = graph.query(prev_query_string)
        self.env.assertEquals(results.result_set[0][0], 0, depth=1)

        query_issue_timestamp_ms = get_unix_timestamp_milliseconds()
        query = """UNWIND (range(0, 10000000)) AS x WITH x AS x WHERE (x / 90000) = 1 RETURN x"""
        waiter = run_concurrently((query), thread_run_query)

        # Wait until the concurrent query (UNWIND) starts execution: until
        # another redis client with the query above issues a query and it
        # starts being executed.
        self.env.assertTrue(self._wait_for_number_of_clients(2), depth=1)
        # Got two clients connected.

        # Wait until the query starts execution and get the statistics.
        info = self._wait_till_queries_start_being_executed(prev=True)
        self.env.assertIsNotNone(info, depth=1)

        waiter.wait()
        results = waiter.get().result_set
        # Validate the GRAPH.QUERY result.
        self.env.assertEquals(results[0][0], 90000)
        # The very last query in the combined output should the query
        # which is currently being executed.
        current_query = list_to_dict(list_to_dict(info)['Queries'][-1])
        self.env.assertGreaterEqual(current_query['Received at'], query_issue_timestamp_ms, depth=1)
        self.env.assertEqual(current_query['Stage'], QUERY_STAGE_EXECUTING, depth=1)
        self.env.assertEqual(current_query['Graph name'], GRAPH_ID, depth=1)
        self.env.assertEqual(current_query['Query'], query, depth=1)

        # The query right before the current should be the last executed
        # (finished) query which we know as we issued it right before the
        # current one.
        prev_query = list_to_dict(list_to_dict(info)['Queries'][-2])
        self.env.assertLessEqual(prev_query['Received at'], query_issue_timestamp_ms, depth=1)
        self.env.assertEqual(prev_query['Stage'], QUERY_STAGE_FINISHED, depth=1)
        self.env.assertEqual(prev_query['Graph name'], GRAPH_ID, depth=1)
        self.env.assertEqual(prev_query['Query'], prev_query_string, depth=1)

    def test07_info_reset_specific(self):
        # Reset a non-existing graph should return an error.
        try:
            query = 'GRAPH.INFO RESET 124878fd'
            results = self.conn.execute_command(query)
            # This should be unreachable.
            assert(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertEquals(str(e), COULDNOT_FIND_GRAPH_ERROR_MESSAGE, depth=1)

        # Reset an existing (current) graph should return a boolean true.
        query = 'GRAPH.INFO RESET %s' % GRAPH_ID
        results = self.conn.execute_command(query)
        self.env.assertEquals(results, 1, depth=1)

# This test is separate as it needs a separate and a non-concurrent context.
class testGraphInfoGetFlow(_testGraphInfoFlowBase):
    QUERY = 'GRAPH.INFO GET %s COUNTS' % GRAPH_ID

    def __init__(self):
        _testGraphInfoFlowBase.__init__(self, Env(decodeResponses=True, moduleArgs='TIMEOUT_MAX 1000'))

    def test01_info_get_specific_counters_successful(self):
        nodes = self._recreate_graph_with_node()

        query = 'GRAPH.INFO RESET %s' % GRAPH_ID
        results = self.conn.execute_command(query)
        self.env.assertEquals(results, 1, depth=1)

        query = 'GRAPH.INFO GET %s' % GRAPH_ID
        results = self.conn.execute_command(query)
        self._assert_info_get_result(results, nodes=nodes, node_labels=1, node_property_names=2)

        results = self.conn.execute_command(self.QUERY)
        self._assert_info_get_result(results, nodes=nodes, node_labels=1, node_property_names=2)
        self._assert_info_get_counts(results)

        # Test all the successful read-only queries:
        successful_readonly_count = 0
        total_query_count = 0
        for kind in ALL_READONLY_KINDS:
            self._run_readonly_query(kind)
            results = self.conn.execute_command(self.QUERY)
            successful_readonly_count += 1
            total_query_count += 1
            self._assert_info_get_result(results, nodes=nodes, node_labels=1, node_property_names=2)
            self._assert_info_get_counts(results, total_query_count=total_query_count, successful_readonly=successful_readonly_count)

        # Test all successful write queries.
        successful_write_count = 0
        for kind in ALL_WRITE_KINDS:
            self._run_write_query(kind)
            results = self.conn.execute_command(self.QUERY)
            successful_write_count += 1
            total_query_count += 1
            nodes += 1
            self._assert_info_get_result(results, nodes=nodes, node_labels=2, node_property_names=2)
            self._assert_info_get_counts(results, total_query_count=total_query_count, successful_readonly=successful_readonly_count, successful_write=successful_write_count)

        query = 'GRAPH.INFO RESET %s' % GRAPH_ID
        results = self.conn.execute_command(query)
        self.env.assertEquals(results, 1, depth=1)
        results = self.conn.execute_command(self.QUERY)
        self._assert_info_get_counts(results)

    def test02_info_get_specific_counters_failed_at_runtime(self):
        nodes = self._recreate_graph_with_node()

        query = 'GRAPH.INFO RESET %s' % GRAPH_ID
        results = self.conn.execute_command(query)
        self.env.assertEquals(results, 1, depth=1)
        total_query_count = 0

        # Test all read queries for fail counting.
        failed_readonly_queries = 0
        for kind in [ReadOnlyQueryKind.RO_QUERY, ReadOnlyQueryKind.QUERY, ReadOnlyQueryKind.PROFILE]:
            try:
                results = self._run_readonly_query(kind, problem_kind=QueryFailureSimulationKind.FAIL_RUNTIME)
                assert False, \
                    f"Shouldn't have reached with the {kind} kind, result: {results}"
            except redis.exceptions.ResponseError as e:
                self.env.assertContains(RUNTIME_FAILURE_MESSAGE, str(e), depth=1)

            results = self.conn.execute_command(self.QUERY)
            failed_readonly_queries += 1
            total_query_count += 1
            self._assert_info_get_result(results, nodes=nodes, node_labels=1, node_property_names=2)
            self._assert_info_get_counts(
                results,
                total_query_count=total_query_count,
                failed_readonly=failed_readonly_queries)

        # TODO fails due to incremented Graph_DeletedNodeCount.
        # # Test all write queries for fail counting.
        # failed_write_queries = 0
        # for kind in ALL_WRITE_KINDS:
        #     try:
        #         results = self._run_write_query(kind, problem_kind=QueryFailureSimulationKind.FAIL_RUNTIME)
        #         assert False, \
        #             f"Shouldn't have reached with the {kind} kind, result: {results}"
        #     except redis.exceptions.ResponseError as e:
        #         self.env.assertContains(RUNTIME_FAILURE_MESSAGE, str(e), depth=1)

        #     results = self.conn.execute_command(self.QUERY)
        #     failed_write_queries += 1
        #     total_query_count += 1
        #     print(f'Query: {self.QUERY}\nResult: {results}')
        #     self._assert_info_get_result(results, nodes=nodes, node_labels=1, node_property_names=2)
        #     self._assert_info_get_counts(
        #         results,
        #         total_query_count=total_query_count,
        #         failed_readonly=failed_readonly_queries,
        #         failed_write=failed_write_queries)

    def test02_info_get_specific_counters_failed_early(self):
        '''
        When a query fails early, we can't record it as we don't know what
        it was about, so we also don't count those, hence the counters are
        expected to be zero.
        '''

        nodes = self._recreate_graph_with_node()

        query = 'GRAPH.INFO RESET %s' % GRAPH_ID
        results = self.conn.execute_command(query)
        self.env.assertEquals(results, 1, depth=1)

        # Test all read queries for fail counting.
        for kind in [ReadOnlyQueryKind.RO_QUERY, ReadOnlyQueryKind.QUERY, ReadOnlyQueryKind.PROFILE]:
            try:
                results = self._run_readonly_query(kind, problem_kind=QueryFailureSimulationKind.FAIL_EARLY)
                assert False, \
                    f"Shouldn't have reached with the {kind} kind, result: {results}"
            except redis.exceptions.ResponseError as e:
                self.env.assertContains(RUNTIME_FAILURE_MESSAGE, str(e), depth=1)

            results = self.conn.execute_command(self.QUERY)
            self._assert_info_get_result(results, nodes=nodes, node_labels=1, node_property_names=2)
            self._assert_info_get_counts(
                results,
                total_query_count=0,
                failed_readonly=0)

        # Test all write queries for fail counting.
        for kind in ALL_WRITE_KINDS:
            try:
                results = self._run_write_query(kind, problem_kind=QueryFailureSimulationKind.FAIL_EARLY)
                assert False, \
                    f"Shouldn't have reached with the {kind} kind, result: {results}"
            except redis.exceptions.ResponseError as e:
                self.env.assertContains(RUNTIME_FAILURE_MESSAGE, str(e), depth=1)

            results = self.conn.execute_command(self.QUERY)
            self._assert_info_get_result(results, nodes=nodes, node_labels=1, node_property_names=2)
            self._assert_info_get_counts(
                results,
                total_query_count=0,
                failed_readonly=0,
                failed_write=0)

    def test03_info_get_specific_counters_timedout(self):
        nodes = self._recreate_graph_with_node()

        query = 'GRAPH.INFO RESET %s' % GRAPH_ID
        results = self.conn.execute_command(query)
        self.env.assertEquals(results, 1, depth=1)
        total_query_count = 0

        # Test all read queries for time out counting.
        timed_out_readonly_queries = 0
        for kind in [ReadOnlyQueryKind.RO_QUERY, ReadOnlyQueryKind.QUERY, ReadOnlyQueryKind.PROFILE]:
            try:
                results = self._run_readonly_query(kind, problem_kind=QueryFailureSimulationKind.TIMEOUT)
                assert False, \
                    f"Shouldn't have reached with the {kind} kind, result: {results}"
            except redis.exceptions.ResponseError as e:
                self.env.assertEquals(str(e), TIMEOUT_ERROR_MESSAGE, depth=1)

            results = self.conn.execute_command(self.QUERY)
            timed_out_readonly_queries += 1
            total_query_count += 1
            self._assert_info_get_result(results, nodes=nodes, node_labels=1, node_property_names=2)
            self._assert_info_get_counts(
                results,
                total_query_count=total_query_count,
                timedout_readonly=timed_out_readonly_queries)

        # Test all write queries for time out counting.
        timed_out_write_queries = 0
        for kind in ALL_WRITE_KINDS:
            try:
                self._run_write_query(kind, problem_kind=QueryFailureSimulationKind.TIMEOUT)
                assert False, \
                    f"Shouldn't have reached with the {kind} kind, result: {results}"
            except redis.exceptions.ResponseError as e:
                self.env.assertEquals(str(e), TIMEOUT_ERROR_MESSAGE, depth=1)

            results = self.conn.execute_command(self.QUERY)
            timed_out_write_queries += 1
            total_query_count += 1
            self._assert_info_get_counts(
                results,
                total_query_count=total_query_count,
                timedout_readonly=timed_out_readonly_queries,
                timedout_write=timed_out_write_queries)

    # TODO
    # 0) Parametrise the tests instead of code duplication.
    # 1) GRAPH.INFO GET * tests.
    #
    # These require some discussion:
    #
    # 2) GRAPH.INFO CONFIG ?
    # 3) GRAPH.INFO INDEXES ?
    # 4) GRAPH.INFO CONSTRAINTS ?
