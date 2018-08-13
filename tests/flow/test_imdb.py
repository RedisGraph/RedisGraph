import os
import sys
import unittest
from redisgraph import Graph

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/../../demo/imdb/')
from .disposableredis import DisposableRedis
from base import FlowTestsBase
import imdb_queries as queries
import imdb_utils

redis_graph = None

def redis():
    return DisposableRedis(loadmodule=os.path.dirname(os.path.abspath(__file__)) + '/../../src/redisgraph.so')

class ImdbFlowTest(FlowTestsBase):
    @classmethod
    def setUpClass(cls):
        print "ImdbFlowTest"
        global redis_graph
        cls.r = redis()
        cls.r.start()
        redis_con = cls.r.client()
        redis_graph = Graph(imdb_utils.graph_name, redis_con)
        imdb_utils.populate_graph(redis_con, redis_graph)

    @classmethod
    def tearDownClass(cls):
        cls.r.stop()

    def test_actors_played_with_nicolas_cage(self):
        global redis_graph
        actual_result = redis_graph.query(queries.actors_played_with_nicolas_cage_query.query)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.actors_played_with_nicolas_cage_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.actors_played_with_nicolas_cage_query)

    def test_find_three_actors_played_with_nicolas_cage(self):
        global redis_graph
        NUM_EXPECTED_RESULTS = 3

        actual_result = redis_graph.query(queries.find_three_actors_played_with_nicolas_cage_query.query)

        # assert result set
        self._assert_actual_results_contained_in_expected_results(
            actual_result,
            queries.find_three_actors_played_with_nicolas_cage_query,
            NUM_EXPECTED_RESULTS)

        # assert query run time
        self._assert_run_time(actual_result, queries.find_three_actors_played_with_nicolas_cage_query)

    def test_actors_played_in_movie_straight_outta_compton(self):
        global redis_graph
        actual_result = redis_graph.query(queries.actors_played_in_movie_straight_outta_compton_query.query)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.actors_played_in_movie_straight_outta_compton_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.actors_played_in_movie_straight_outta_compton_query)

    def test_actors_over_50_that_played_in_blockbusters(self):
        global redis_graph
        actual_result = redis_graph.query(queries.actors_over_50_that_played_in_blockbusters_query.query)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.actors_over_50_that_played_in_blockbusters_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.actors_over_50_that_played_in_blockbusters_query)

    def test_actors_played_in_bad_drama_or_comedy(self):
        global redis_graph
        actual_result = redis_graph.query(queries.actors_played_in_bad_drama_or_comedy_query.query)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.actors_played_in_bad_drama_or_comedy_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.actors_played_in_bad_drama_or_comedy_query)

    def test_young_actors_played_with_cameron_diaz(self):
        global redis_graph
        actual_result = redis_graph.query(queries.young_actors_played_with_cameron_diaz_query.query)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.young_actors_played_with_cameron_diaz_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.young_actors_played_with_cameron_diaz_query)

    def test_actors_played_with_cameron_diaz_and_younger_than_her(self):
        global redis_graph
        actual_result = redis_graph.query(queries.actors_played_with_cameron_diaz_and_younger_than_her_query.query)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.actors_played_with_cameron_diaz_and_younger_than_her_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.actors_played_with_cameron_diaz_and_younger_than_her_query)

    def test_sum_and_average_age_of_straight_outta_compton_cast(self):
        global redis_graph
        actual_result = redis_graph.query(queries.sum_and_average_age_of_straight_outta_compton_cast_query.query)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.sum_and_average_age_of_straight_outta_compton_cast_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.sum_and_average_age_of_straight_outta_compton_cast_query)

    def test_how_many_movies_cameron_diaz_played(self):
        global redis_graph
        actual_result = redis_graph.query(queries.how_many_movies_cameron_diaz_played_query.query)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.how_many_movies_cameron_diaz_played_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.how_many_movies_cameron_diaz_played_query)

    def test_find_ten_oldest_actors(self):
        global redis_graph
        actual_result = redis_graph.query(queries.find_ten_oldest_actors_query.query)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.find_ten_oldest_actors_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.find_ten_oldest_actors_query)

    def test_index_scan_actors_over_85(self):
        global redis_graph

        # Execute this command directly, as its response does not contain the result set that
        # 'redis_graph.query()' expects
        redis_graph.redis_con.execute_command("GRAPH.QUERY", redis_graph.name, "CREATE INDEX ON :actor(age)")
        execution_plan = redis_graph.execution_plan(queries.actors_over_85_index_scan.query)
        self.assertIn('Index Scan', execution_plan)

        actual_result = redis_graph.query(queries.actors_over_85_index_scan.query)

        redis_graph.redis_con.execute_command("GRAPH.QUERY", redis_graph.name, "DROP INDEX ON :actor(age)")

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.actors_over_85_index_scan)

        # assert query run time
        self._assert_run_time(actual_result, queries.actors_over_85_index_scan)

if __name__ == '__main__':
    unittest.main()
