import os
import sys
import unittest
from redisgraph import Graph

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/../../demo/imdb/')
from base import FlowTestsBase
import imdb_queries as queries
import imdb_utils


class ImdbFlowTest(FlowTestsBase):

    def test_actors_played_with_nicolas_cage(self):
        with self.redis() as redis_con:
            redis_graph = Graph(imdb_utils.graph_name, redis_con)
            imdb_utils.populate_graph(redis_con, redis_graph)
            actual_result = redis_graph.query(queries.actors_played_with_nicolas_cage_query.query)

            # assert result set
            self._assert_only_expected_resuls_are_in_actual_results(
                actual_result,
                queries.actors_played_with_nicolas_cage_query)

            # assert query run time
            self._assert_run_time(actual_result, queries.actors_played_with_nicolas_cage_query)

    def test_find_three_actors_played_with_nicolas_cage(self):
        NUM_EXPECTED_RESULTS = 3

        with self.redis() as redis_con:
            redis_graph = Graph(imdb_utils.graph_name, redis_con)
            imdb_utils.populate_graph(redis_con, redis_graph)
            actual_result = redis_graph.query(queries.find_three_actors_played_with_nicolas_cage_query.query)

            # assert result set
            self._assert_actual_results_contained_in_expected_results(
                actual_result,
                queries.find_three_actors_played_with_nicolas_cage_query,
                NUM_EXPECTED_RESULTS)

            # assert query run time
            self._assert_run_time(actual_result, queries.find_three_actors_played_with_nicolas_cage_query)

    def test_actors_played_in_movie_straight_outta_compton(self):
        with self.redis() as redis_con:
            redis_graph = Graph(imdb_utils.graph_name, redis_con)
            imdb_utils.populate_graph(redis_con, redis_graph)
            actual_result = redis_graph.query(queries.actors_played_in_movie_straight_outta_compton_query.query)

            # assert result set
            self._assert_only_expected_resuls_are_in_actual_results(
                actual_result,
                queries.actors_played_in_movie_straight_outta_compton_query)

            # assert query run time
            self._assert_run_time(actual_result, queries.actors_played_in_movie_straight_outta_compton_query)


    def test_actors_over_50_that_played_in_blockbusters(self):
        with self.redis() as redis_con:
            redis_graph = Graph(imdb_utils.graph_name, redis_con)
            imdb_utils.populate_graph(redis_con, redis_graph)
            actual_result = redis_graph.query(queries.actors_over_50_that_played_in_blockbusters_query.query)

            # assert result set
            self._assert_only_expected_resuls_are_in_actual_results(
                actual_result,
                queries.actors_over_50_that_played_in_blockbusters_query)

            # assert query run time
            self._assert_run_time(actual_result, queries.actors_over_50_that_played_in_blockbusters_query)

    def test_actors_played_in_bad_drama_or_comedy(self):
        with self.redis() as redis_con:
            redis_graph = Graph(imdb_utils.graph_name, redis_con)
            imdb_utils.populate_graph(redis_con, redis_graph)
            actual_result = redis_graph.query(queries.actors_played_in_bad_drama_or_comedy_query.query)

            # assert result set
            self._assert_only_expected_resuls_are_in_actual_results(
                actual_result,
                queries.actors_played_in_bad_drama_or_comedy_query)

            # assert query run time
            self._assert_run_time(actual_result, queries.actors_played_in_bad_drama_or_comedy_query)

    def test_young_actors_played_with_cameron_diaz(self):
        with self.redis() as redis_con:
            redis_graph = Graph(imdb_utils.graph_name, redis_con)
            imdb_utils.populate_graph(redis_con, redis_graph)
            actual_result = redis_graph.query(queries.young_actors_played_with_cameron_diaz_query.query)

            # assert result set
            self._assert_only_expected_resuls_are_in_actual_results(
                actual_result,
                queries.young_actors_played_with_cameron_diaz_query)

            # assert query run time
            self._assert_run_time(actual_result, queries.young_actors_played_with_cameron_diaz_query)

    def test_actors_played_with_cameron_diaz_and_younger_than_her(self):
        with self.redis() as redis_con:
            redis_graph = Graph(imdb_utils.graph_name, redis_con)
            imdb_utils.populate_graph(redis_con, redis_graph)
            actual_result = redis_graph.query(queries.actors_played_with_cameron_diaz_and_younger_than_her_query.query)

            # assert result set
            self._assert_only_expected_resuls_are_in_actual_results(
                actual_result,
                queries.actors_played_with_cameron_diaz_and_younger_than_her_query)

            # assert query run time
            self._assert_run_time(actual_result, queries.actors_played_with_cameron_diaz_and_younger_than_her_query)

    def test_sum_and_average_age_of_straight_outta_compton_cast(self):
        with self.redis() as redis_con:
            redis_graph = Graph(imdb_utils.graph_name, redis_con)
            imdb_utils.populate_graph(redis_con, redis_graph)
            actual_result = redis_graph.query(queries.sum_and_average_age_of_straight_outta_compton_cast_query.query)

            # assert result set
            self._assert_only_expected_resuls_are_in_actual_results(
                actual_result,
                queries.sum_and_average_age_of_straight_outta_compton_cast_query)

            # assert query run time
            self._assert_run_time(actual_result, queries.sum_and_average_age_of_straight_outta_compton_cast_query)

    def test_how_many_movies_cameron_diaz_played(self):
        with self.redis() as redis_con:
            redis_graph = Graph(imdb_utils.graph_name, redis_con)
            imdb_utils.populate_graph(redis_con, redis_graph)
            actual_result = redis_graph.query(queries.how_many_movies_cameron_diaz_played_query.query)

            # assert result set
            self._assert_only_expected_resuls_are_in_actual_results(
                actual_result,
                queries.how_many_movies_cameron_diaz_played_query)

            # assert query run time
            self._assert_run_time(actual_result, queries.how_many_movies_cameron_diaz_played_query)

    def test_find_ten_oldest_actors(self):
        with self.redis() as redis_con:
            redis_graph = Graph(imdb_utils.graph_name, redis_con)
            imdb_utils.populate_graph(redis_con, redis_graph)
            actual_result = redis_graph.query(queries.find_ten_oldest_actors_query.query)

            # assert result set
            self._assert_only_expected_resuls_are_in_actual_results(
                actual_result,
                queries.find_ten_oldest_actors_query)

            # assert query run time
            self._assert_run_time(actual_result, queries.find_ten_oldest_actors_query)


if __name__ == '__main__':
    unittest.main()
