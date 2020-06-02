import os
import sys
from RLTest import Env
from redisgraph import Graph
from base import FlowTestsBase

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
from reversepattern import ReversePattern

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/../../demo/imdb/')
import imdb_queries
import imdb_utils

imdb = None
queries = None
redis_graph = None

class testImdbFlow(FlowTestsBase):
    def __init__(self):
        self.env = Env()

    def setUp(self):
        global imdb
        global queries
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(imdb_utils.graph_name, redis_con)
        actors, movies = imdb_utils.populate_graph(redis_con, redis_graph)
        imdb = imdb_queries.IMDBQueries(actors, movies)
        queries = imdb.queries()

    def tearDown(self):
        self.env.cmd('flushall')

    def assert_reversed_pattern(self, query, resultset):
        # Test reversed pattern query.
        reversed_query = ReversePattern().reverse_query_pattern(query)
        # print "reversed_query: %s" % reversed_query
        actual_result = redis_graph.query(reversed_query)

        # assert result set
        self.env.assertEqual(resultset.result_set, actual_result.result_set)

        # assert query run time
        self._assert_equalish(resultset.run_time_ms, actual_result.run_time_ms)

    def test_imdb(self):
        for q in queries:
            query = q.query
            actual_result = redis_graph.query(query)

            # assert result set
            self._assert_only_expected_results_are_in_actual_results(actual_result, q)

            # assert query run time
            self._assert_run_time(actual_result, q)

            if q.reversible:
                # assert reversed pattern.
                self.assert_reversed_pattern(query, actual_result)
    
    def test_index_scan_actors_over_85(self):
        global redis_graph

        # Execute this command directly, as its response does not contain the result set that
        # 'redis_graph.query()' expects
        redis_con = self.env.getConnection()
        res = redis_con.execute_command("GRAPH.QUERY", redis_graph.name, "CREATE INDEX ON :actor(age)")

        q = imdb.actors_over_85_index_scan.query
        execution_plan = redis_graph.execution_plan(q)
        self.env.assertIn('Index Scan', execution_plan)

        actual_result = redis_graph.query(q)

        redis_con.execute_command("GRAPH.QUERY", redis_graph.name, "DROP INDEX ON :actor(age)")

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            imdb.actors_over_85_index_scan)

        # assert query run time
        self._assert_run_time(actual_result, imdb.actors_over_85_index_scan)

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test_index_scan_eighties_movies(self):
        global redis_graph

        # Execute this command directly, as its response does not contain the result set that
        # 'redis_graph.query()' expects
        redis_graph.redis_con.execute_command("GRAPH.QUERY", redis_graph.name, "CREATE INDEX ON :movie(year)")
        q = imdb.eighties_movies_index_scan.query
        execution_plan = redis_graph.execution_plan(q)
        self.env.assertIn('Index Scan', execution_plan)

        actual_result = redis_graph.query(q)

        redis_graph.redis_con.execute_command("GRAPH.QUERY", redis_graph.name, "DROP INDEX ON :movie(year)")

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            imdb.eighties_movies_index_scan)

        # assert query run time
        self._assert_run_time(actual_result, imdb.eighties_movies_index_scan)

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)
