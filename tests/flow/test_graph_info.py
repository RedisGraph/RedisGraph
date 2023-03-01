# GRAPH.INFO mostly being useful only concurrently, requires concurrent tests.
from common import *
from pathos.pools import ProcessPool as Pool
import csv
import time
from click.testing import CliRunner
from redisgraph_bulk_loader.bulk_insert import bulk_insert

from common import *
import time
from enum import Enum

GRAPH_ID = "GRAPH_INFO_TEST"
GRAPH_ID_2 = "GRAPH_INFO_TEST_2"


# GRAPH.INFO commands
INFO_QUERIES_CURRENT_COMMAND = 'GRAPH.INFO QUERIES CURRENT'
INFO_QUERIES_PREV_COMMAND = 'GRAPH.INFO QUERIES PREV 100 --compact'
INFO_QUERIES_CURRENT_PREV_COMMAND = 'GRAPH.INFO QUERIES CURRENT PREV 100'

# Error messages
COULDNOT_FIND_GRAPH_ERROR_MESSAGE = "Couldn't find the specified graph"
NO_GRAPH_NAME_SPECIFIED = "No graph name was specified"
TIMEOUT_ERROR_MESSAGE = "Query timed out"
RUNTIME_FAILURE_MESSAGE = "Division by zero"
UNKNOWN_SUBCOMMAND = 'Unknown subcommand.'
COMMAND_IS_DISABLED = "The info tracking is disabled."

# Keys
CURRENT_MAXIMUM_WAIT_DURATION_KEY_NAME = 'Current maximum query wait duration'
GLOBAL_INFO_KEY_NAME = 'Global info'
QUERY_STAGE_EXECUTING = 1
QUERY_STAGE_FINISHED = 3

# Other
LONG_CALCULATION_QUERY = """UNWIND (range(0, 10000000)) AS x WITH x AS x WHERE (x / 90000) = 1 RETURN x"""

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
    def _delete_graph(self, name=GRAPH_ID):
        graph = Graph(self.conn, name)
        try:
            graph.delete()
        except redis.exceptions.ResponseError:
            pass

    def _delete_graphs(self):
        self._delete_graph(GRAPH_ID)
        self._delete_graph(GRAPH_ID_2)

    def _create_graph_filled(self, name=GRAPH_ID):
        graph = Graph(self.conn, name)

        for i in range(0, 2):
            graph.add_node(Node(label='Person', properties={'age': i}))

        graph.commit()
        return i + 1

    def _create_graph_empty(self, name=GRAPH_ID):
        self.conn.execute_command('GRAPH.QUERY', name, 'RETURN 1')

    def _recreate_graph_empty(self, name=GRAPH_ID):
        self._delete_graph(name)
        self._create_graph_empty(name)

    def _recreate_graph_with_node(self, name=GRAPH_ID):
        self._delete_graph(name)
        return self._create_graph_filled(name)

    def _reset_graph_info_stats(self, name=GRAPH_ID):
        query = INFO_RESET_SPECIFIC_COMMAND_TEMPLATE % GRAPH_ID
        results = self.conn.execute_command(query)
        self.env.assertEquals(results, 1, depth=1)

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
        wait_step = 0.01
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
    

    # Runs a read-only query. A read-only query isn't just based on whether
    # it is run via GRAPH.RO_QUERY but it can also be an ordinary GRAPH.QUERY
    # that just doesn't change anything. In the code we analyse the AST of the
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
            # Parsing error is an example of a "fail early" thing.
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
            query = 'UNWIND (range(0, 10000000)) AS x WITH x CREATE(b:Book { id: x })'
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
        info = self._wait_till_queries_start_being_executed(timeout=10)
        self.env.assertIsNotNone(info)

        waiter.wait()
        results = waiter.get().result_set
        # Validate the GRAPH.QUERY result.
        self.env.assertEquals(results[0][0], 90000)
        self._assert_one_executing_query(info, query, assert_receive_time=query_issue_timestamp_ms)
