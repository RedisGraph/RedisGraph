# GRAPH.INFO being useful only concurrently, requires concurrent tests.
import asyncio
from common import *
from pathos.pools import ProcessPool as Pool
from pathos.helpers import mp as pathos_multiprocess

from common import *
import time

GRAPH_ID = "G"
INFO_QUERIES_COMMAND = 'GRAPH.INFO QUERIES --compact'
INFO_GET_COMMAND_TEMPLATE = 'GRAPH.INFO GET %s --compact'
INFO_RESET_ALL_COMMAND = 'GRAPH.INFO RESET * --compact'


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

def get_total_executing_queries_from_info_cmd_result(info):
    # [1][5] = 'Global info' => 'Total executing queries count'
    return info[1][5]


class testGraphInfoFlow(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)
        # skip test if we're running under Valgrind
        if self.env.envRunner.debugger is not None:
            self.env.skip() # valgrind is not working correctly with multi processing
        self.conn = self.env.getConnection()
        graph = Graph(self.conn, GRAPH_ID)
        graph.add_node(Node(label='person'))
        graph.commit()

    # Validate the GRAPH.INFO result: should contain the queries specified,
    # and exactly thath number of queries being executed.
    def _assert_executing_queries(self, info, queries=[]):
        # if the info object is passed directly as a result, convert it to
        # a dictionary for easier and consistent assertions.
        if not isinstance(info, dict):
            info = list_to_dict(info)
        # It has already accounted for the graph creation query, which might
        # have taken longer than a millisecond.
        self.env.assertGreaterEqual(info['Global info']['Max Query Pipeline Time (milliseconds)'], 0, depth=1)
        # No other queries has finished yet.
        # No queries are waiting to be executed as the only query we
        # launched must have already started being executed.
        self.env.assertEquals(info['Global info']['Total waiting queries count'], 0, depth=1)
        # The current query is being executed.
        executing_queries_count = len(queries)
        self.env.assertEquals(info['Global info']['Total executing queries count'], executing_queries_count, depth=1)
        # As there are no other queries, no query is reporting.
        self.env.assertEquals(info['Global info']['Total reporting queries count'], 0, depth=1)
        # Check that the only query being currently executed is ours.
        for i in range(0, executing_queries_count):
            executing_query = list_to_dict(info['Per-graph data'][GRAPH_ID]['Executing queries'][i])
            self.env.assertEquals(executing_query['Query'], queries[i], depth=1)
            self.env.assertGreaterEqual(executing_query['Current total time (milliseconds)'], 0, depth=1)
            self.env.assertGreaterEqual(executing_query['Current wait time (milliseconds)'], 0, depth=1)
            self.env.assertGreaterEqual(executing_query['Current execution time (milliseconds)'], 0, depth=1)
            self.env.assertEqual(executing_query['Current reporting time (milliseconds)'], 0, depth=1)
            self.env.assertEqual(len(info['Per-graph data'][GRAPH_ID]['Reporting queries']), 0, depth=1)

    def _assert_one_executing_query(self, info, query):
        queries = []
        if query is not None:
            queries.append(query)
        self._assert_executing_queries(info, queries)

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

    def _wait_till_queries_start_being_executed(self, query_count=1, timeout=2):
        wait_step = 0.1
        waited_time = 0
        while True:
            info = self.conn.execute_command(INFO_QUERIES_COMMAND)
            count = get_total_executing_queries_from_info_cmd_result(info)
            if count >= query_count:
                # Found the query being executed.
                return info
            time.sleep(wait_step)
            waited_time += wait_step
            if waited_time >= timeout:
                return None

    def test01_empty_graph_info(self):
        info = self.conn.execute_command(INFO_QUERIES_COMMAND)
        self._assert_one_executing_query(info, None)

    def test02_long_query_is_recorded_as_being_executed(self):
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
        self._assert_one_executing_query(info, query)

    def test03_graph_info_reset_all(self):
        info = self.conn.execute_command(INFO_QUERIES_COMMAND)
        self._assert_one_executing_query(info, None)

        query = """UNWIND (range(0, 1000000)) AS x WITH x AS x WHERE (x / 90000) = 1 RETURN x"""
        graph = Graph(self.conn, GRAPH_ID)
        results = graph.query(query)
        self.env.assertEquals(results.result_set[0][0], 90000, depth=1)
        query_time = results.statistics['internal execution time']

        info = list_to_dict(self.conn.execute_command(INFO_QUERIES_COMMAND))
        self.env.assertGreaterEqual(info['Global info']['Max Query Pipeline Time (milliseconds)'], query_time, depth=1)

        reset = self.conn.execute_command(INFO_RESET_ALL_COMMAND)
        self.env.assertTrue(reset, depth=1)

        info = list_to_dict(self.conn.execute_command(INFO_QUERIES_COMMAND))
        self.env.assertEqual(info['Global info']['Max Query Pipeline Time (milliseconds)'], 0, depth=1)

    def _assert_info_get_result(self, result,
        nodes=0, relationships=0,
        node_labels=0, relationship_types=0,
        node_indices=0, relationship_indices=0,
        node_property_names=0, edge_property_names=0):
        info = list_to_dict(result)
        self.env.assertEqual(info['Number of nodes'], nodes)
        self.env.assertEqual(info['Number of relationships'], relationships)
        self.env.assertEqual(info['Number of node labels'], node_labels)
        self.env.assertEqual(info['Number of relationship types'], relationship_types)
        self.env.assertEqual(info['Number of node indices'], node_indices)
        self.env.assertEqual(info['Number of relationship indices'], relationship_indices)
        self.env.assertEqual(info['Total number of node property names'], node_property_names)
        self.env.assertEqual(info['Total number of edge property names'], edge_property_names)

    def test03_graph_info_get_current_graph(self):
        info = self.conn.execute_command(INFO_GET_COMMAND_TEMPLATE % GRAPH_ID)
        self._assert_info_get_result(info, nodes=1, node_labels=1)
        query = """MATCH (p:person) CREATE (p2:person { Name: 'Victor', Country: 'The Netherlands' })-[e:knows { Since_Year: '1970'}]->(p)"""
        graph = Graph(self.conn, GRAPH_ID)
        result = graph.query(query)
        self.env.assertEquals(result.nodes_created, 1, depth=1)
        self.env.assertEquals(result.relationships_created, 1, depth=1)
        self.env.assertEquals(result.properties_set, 3, depth=1)

        info = self.conn.execute_command(INFO_GET_COMMAND_TEMPLATE % GRAPH_ID)
        self._assert_info_get_result(info, nodes=2, node_labels=1, relationships=1, relationship_types=1, node_property_names=2, edge_property_names=1)

    # TODO
    # 1) Reset some graphs, not all, check the output.
    # 2) GRAPH.INFO GET * tests.
    # 3) Test more fields of the output.
