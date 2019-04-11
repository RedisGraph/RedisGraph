import os
import sys
import unittest
import redis
from redisgraph import Graph

from .base import FlowTestsBase
from .reversepattern import ReversePattern

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/../../demo/imdb/')
import imdb_queries as queries
import imdb_utils

redis_graph = None
redis_con = None
dis_redis = None

def get_redis():
    global dis_redis
    conn = redis.Redis()
    try:
        conn.ping()
        # Assuming RedisGraph is loaded.
    except redis.exceptions.ConnectionError:
        # Bring up our own redis-server instance.
        from .redis_base import DisposableRedis
        dis_redis = DisposableRedis(loadmodule=os.path.dirname(os.path.abspath(__file__)) + '/../../src/redisgraph.so')
        dis_redis.start()
        conn = dis_redis.client()
    return conn

class ImdbFlowTest(FlowTestsBase):
    @classmethod
    def setUpClass(cls):
        print "ImdbFlowTest"
        global redis_graph
        global redis_con
        redis_con = get_redis()
        redis_graph = Graph(imdb_utils.graph_name, redis_con)
        imdb_utils.populate_graph(redis_con, redis_graph)

    @classmethod
    def tearDownClass(cls):
        if dis_redis is not None:
            dis_redis.stop()

    def assert_reversed_pattern(self, query, resultset):
        # Test reversed pattern query.
        reversed_query = ReversePattern().reverse_query_pattern(query)
        # print "reversed_query: %s" % reversed_query
        actual_result = redis_graph.query(reversed_query)

        # assert result set
        self.assertEqual(resultset.result_set, actual_result.result_set)

        # assert query run time
        self._assert_equalish(resultset.run_time_ms, actual_result.run_time_ms)

    def test_number_of_actors(self):
        global redis_graph
        q = queries.number_of_actors_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.number_of_actors_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.number_of_actors_query)

        # assert execution plan
        execution_plan = redis_graph.execution_plan(q)
        self.assertNotIn("Aggregate", execution_plan)
        self.assertNotIn("Node By Label Scan", execution_plan)
    
    def test_actors_played_with_nicolas_cage(self):
        global redis_graph
        q = queries.actors_played_with_nicolas_cage_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.actors_played_with_nicolas_cage_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.actors_played_with_nicolas_cage_query)

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test_find_three_actors_played_with_nicolas_cage(self):
        global redis_graph
        NUM_EXPECTED_RESULTS = 3

        q = queries.find_three_actors_played_with_nicolas_cage_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_actual_results_contained_in_expected_results(
            actual_result,
            queries.find_three_actors_played_with_nicolas_cage_query,
            NUM_EXPECTED_RESULTS)

        # assert query run time
        self._assert_run_time(actual_result, queries.find_three_actors_played_with_nicolas_cage_query)
        
        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test_actors_played_in_movie_straight_outta_compton(self):
        global redis_graph
        q = queries.actors_played_in_movie_straight_outta_compton_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.actors_played_in_movie_straight_outta_compton_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.actors_played_in_movie_straight_outta_compton_query)

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test_actors_over_50_that_played_in_blockbusters(self):
        global redis_graph
        q = queries.actors_over_50_that_played_in_blockbusters_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.actors_over_50_that_played_in_blockbusters_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.actors_over_50_that_played_in_blockbusters_query)

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test_actors_played_in_bad_drama_or_comedy(self):
        global redis_graph
        q = queries.actors_played_in_bad_drama_or_comedy_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.actors_played_in_bad_drama_or_comedy_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.actors_played_in_bad_drama_or_comedy_query)

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test_young_actors_played_with_cameron_diaz(self):
        global redis_graph
        q = queries.young_actors_played_with_cameron_diaz_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.young_actors_played_with_cameron_diaz_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.young_actors_played_with_cameron_diaz_query)

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test_actors_played_with_cameron_diaz_and_younger_than_her(self):
        global redis_graph
        q = queries.actors_played_with_cameron_diaz_and_younger_than_her_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.actors_played_with_cameron_diaz_and_younger_than_her_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.actors_played_with_cameron_diaz_and_younger_than_her_query)

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test_sum_and_average_age_of_straight_outta_compton_cast(self):
        global redis_graph
        q = queries.sum_and_average_age_of_straight_outta_compton_cast_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.sum_and_average_age_of_straight_outta_compton_cast_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.sum_and_average_age_of_straight_outta_compton_cast_query)

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test_how_many_movies_cameron_diaz_played(self):
        global redis_graph
        q = queries.how_many_movies_cameron_diaz_played_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.how_many_movies_cameron_diaz_played_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.how_many_movies_cameron_diaz_played_query)

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test_find_ten_oldest_actors(self):
        global redis_graph
        q = queries.find_ten_oldest_actors_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.find_ten_oldest_actors_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.find_ten_oldest_actors_query)

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test_index_scan_actors_over_85(self):
        global redis_graph

        # Execute this command directly, as its response does not contain the result set that
        # 'redis_graph.query()' expects
        redis_graph.redis_con.execute_command("GRAPH.QUERY", redis_graph.name, "CREATE INDEX ON :actor(age)")
        q = queries.actors_over_85_index_scan.query
        execution_plan = redis_graph.execution_plan(q)
        self.assertIn('Index Scan', execution_plan)

        actual_result = redis_graph.query(q)

        redis_graph.redis_con.execute_command("GRAPH.QUERY", redis_graph.name, "DROP INDEX ON :actor(age)")

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.actors_over_85_index_scan)

        # assert query run time
        self._assert_run_time(actual_result, queries.actors_over_85_index_scan)

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test_index_scan_eighties_movies(self):
        global redis_graph

        # Execute this command directly, as its response does not contain the result set that
        # 'redis_graph.query()' expects
        redis_graph.redis_con.execute_command("GRAPH.QUERY", redis_graph.name, "CREATE INDEX ON :movie(year)")
        q = queries.eighties_movies_index_scan.query
        execution_plan = redis_graph.execution_plan(q)
        self.assertIn('Index Scan', execution_plan)

        actual_result = redis_graph.query(q)

        redis_graph.redis_con.execute_command("GRAPH.QUERY", redis_graph.name, "DROP INDEX ON :movie(year)")

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.eighties_movies_index_scan)

        # assert query run time
        self._assert_run_time(actual_result, queries.eighties_movies_index_scan)

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test_find_titles_starting_with_american(self):
        global redis_graph
        q = queries.find_titles_starting_with_american_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.find_titles_starting_with_american_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.find_titles_starting_with_american_query)

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

if __name__ == '__main__':
    unittest.main()
