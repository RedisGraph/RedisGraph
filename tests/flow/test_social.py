import os
import sys
import unittest
from redisgraph import Graph

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/../../demo/social/')
from base import FlowTestsBase
import social_queries as queries
import social_utils


class SocialFlowTest(FlowTestsBase):

    def test_my_friends(self):
        with self.redis() as redis_con:
            redis_graph = Graph(social_utils.graph_name, redis_con)
            social_utils.populate_graph(redis_con, redis_graph)
            actual_result = redis_graph.query(queries.my_friends_query.query)

            # assert result set
            self._assert_only_expected_resuls_are_in_actual_results(
                actual_result,
                queries.my_friends_query)

            # assert query run time
            self._assert_run_time(actual_result, queries.my_friends_query)

    def test_friends_of_friends(self):
        with self.redis() as redis_con:
            redis_graph = Graph(social_utils.graph_name, redis_con)
            social_utils.populate_graph(redis_con, redis_graph)
            actual_result = redis_graph.query(queries.friends_of_friends_query.query)

            # assert result set
            self._assert_only_expected_resuls_are_in_actual_results(
                actual_result,
                queries.friends_of_friends_query)

            # assert query run time
            self._assert_run_time(actual_result, queries.friends_of_friends_query)

    def test_friends_of_friends_single_and_over_30(self):
        with self.redis() as redis_con:
            redis_graph = Graph(social_utils.graph_name, redis_con)
            social_utils.populate_graph(redis_con, redis_graph)
            actual_result = redis_graph.query(queries.friends_of_friends_single_and_over_30_query.query)

            # assert result set
            self._assert_only_expected_resuls_are_in_actual_results(
                actual_result,
                queries.friends_of_friends_single_and_over_30_query)

            # assert query run time
            self._assert_run_time(actual_result, queries.friends_of_friends_single_and_over_30_query)

    def test_friends_of_friends_visited_amsterdam_and_single(self):
        with self.redis() as redis_con:
            redis_graph = Graph(social_utils.graph_name, redis_con)
            social_utils.populate_graph(redis_con, redis_graph)
            actual_result = redis_graph.query(queries.friends_of_friends_visited_amsterdam_and_single_query.query)

            # assert result set
            self._assert_only_expected_resuls_are_in_actual_results(
                actual_result,
                queries.friends_of_friends_visited_amsterdam_and_single_query)

            # assert query run time
            self._assert_run_time(actual_result, queries.friends_of_friends_visited_amsterdam_and_single_query)

    def test_friends_visited_same_places_as_me(self):
        with self.redis() as redis_con:
            redis_graph = Graph(social_utils.graph_name, redis_con)
            social_utils.populate_graph(redis_con, redis_graph)
            actual_result = redis_graph.query(queries.friends_visited_same_places_as_me_query.query)

            # assert result set
            self._assert_only_expected_resuls_are_in_actual_results(
                actual_result,
                queries.friends_visited_same_places_as_me_query)

            # assert query run time
            self._assert_run_time(actual_result, queries.friends_visited_same_places_as_me_query)

    def test_friends_older_than_me(self):
        with self.redis() as redis_con:
            redis_graph = Graph(social_utils.graph_name, redis_con)
            social_utils.populate_graph(redis_con, redis_graph)
            actual_result = redis_graph.query(queries.friends_older_than_me_query.query)

            # assert result set
            self._assert_only_expected_resuls_are_in_actual_results(
                actual_result,
                queries.friends_older_than_me_query)

            # assert query run time
            self._assert_run_time(actual_result, queries.friends_older_than_me_query)

    def test_how_many_countries_each_friend_visited(self):
        with self.redis() as redis_con:
            redis_graph = Graph(social_utils.graph_name, redis_con)
            social_utils.populate_graph(redis_con, redis_graph)
            actual_result = redis_graph.query(queries.how_many_countries_each_friend_visited_query.query)

            # assert result set
            self._assert_only_expected_resuls_are_in_actual_results(
                actual_result,
                queries.how_many_countries_each_friend_visited_query)

            # assert query run time
            self._assert_run_time(actual_result, queries.how_many_countries_each_friend_visited_query)

    def test_friends_age_statistics(self):
        with self.redis() as redis_con:
            redis_graph = Graph(social_utils.graph_name, redis_con)
            social_utils.populate_graph(redis_con, redis_graph)
            actual_result = redis_graph.query(queries.friends_age_statistics_query.query)

            # assert result set
            self._assert_only_expected_resuls_are_in_actual_results(
                actual_result,
                queries.friends_age_statistics_query)

            # assert query run time
            self._assert_run_time(actual_result, queries.friends_age_statistics_query)

    def test_visit_purpose_of_each_country_i_visited(self):
        with self.redis() as redis_con:
            redis_graph = Graph(social_utils.graph_name, redis_con)
            social_utils.populate_graph(redis_con, redis_graph)
            actual_result = redis_graph.query(queries.visit_purpose_of_each_country_i_visited_query.query)

            # assert result set
            self._assert_only_expected_resuls_are_in_actual_results(
                actual_result,
                queries.visit_purpose_of_each_country_i_visited_query)

            # assert query run time
            self._assert_run_time(actual_result, queries.visit_purpose_of_each_country_i_visited_query)

    def test_who_was_on_business_trip(self):
        with self.redis() as redis_con:
            redis_graph = Graph(social_utils.graph_name, redis_con)
            social_utils.populate_graph(redis_con, redis_graph)
            actual_result = redis_graph.query(queries.who_was_on_business_trip_query.query)

            # assert result set
            self._assert_only_expected_resuls_are_in_actual_results(
                actual_result,
                queries.who_was_on_business_trip_query)

            # assert query run time
            self._assert_run_time(actual_result, queries.who_was_on_business_trip_query)

    def test_number_of_vacations_per_person(self):
        NUM_EXPECTED_RESULTS = 6

        with self.redis() as redis_con:
            redis_graph = Graph(social_utils.graph_name, redis_con)
            social_utils.populate_graph(redis_con, redis_graph)
            actual_result = redis_graph.query(queries.number_of_vacations_per_person_query.query)

            # assert result set
            self._assert_actual_results_contained_in_expected_results(
                actual_result,
                queries.number_of_vacations_per_person_query,
                NUM_EXPECTED_RESULTS)

            # assert query run time
            self._assert_run_time(actual_result, queries.number_of_vacations_per_person_query)


if __name__ == '__main__':
    unittest.main()
