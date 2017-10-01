import sys
import unittest
from redisgraph import Graph

sys.path.append('.')
sys.path.append('../../Demo/social/')
from base import FlowTestsBase
import queries
import social_utils


class SocialFlowTest(FlowTestsBase):

    @classmethod
    def setUpClass(cls):
        super(SocialFlowTest, cls).setUpClass()
        cls.redis_graph = Graph(social_utils.graph_name, cls.redis_con)
        social_utils.populate_graph(cls.redis_con, cls.redis_graph)

    def test_my_friends(self):
        actual_result = self.redis_graph.query(queries.my_friends_query.query)

        # assert result set
        self._assert_only_expected_resuls_are_in_actual_results(
            actual_result,
            queries.my_friends_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.my_friends_query)

    def test_friends_of_friends(self):
        actual_result = self.redis_graph.query(queries.friends_of_friends_query.query)

        # assert result set
        self._assert_only_expected_resuls_are_in_actual_results(
            actual_result,
            queries.friends_of_friends_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.friends_of_friends_query)

    def test_friends_of_friends_single_and_over_30(self):
        actual_result = self.redis_graph.query(queries.friends_of_friends_single_and_over_30_query.query)

        # assert result set
        self._assert_only_expected_resuls_are_in_actual_results(
            actual_result,
            queries.friends_of_friends_single_and_over_30_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.friends_of_friends_single_and_over_30_query)

    def test_friends_of_friends_visited_amsterdam_and_single(self):
        actual_result = self.redis_graph.query(queries.friends_of_friends_visited_amsterdam_and_single_query.query)

        # assert result set
        self._assert_only_expected_resuls_are_in_actual_results(
            actual_result,
            queries.friends_of_friends_visited_amsterdam_and_single_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.friends_of_friends_visited_amsterdam_and_single_query)

    def test_friends_visited_same_places_as_me(self):
        actual_result = self.redis_graph.query(queries.friends_visited_same_places_as_me_query.query)

        # assert result set
        self._assert_only_expected_resuls_are_in_actual_results(
            actual_result,
            queries.friends_visited_same_places_as_me_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.friends_visited_same_places_as_me_query)

    def test_friends_older_than_me(self):
        actual_result = self.redis_graph.query(queries.friends_older_than_me_query.query)

        # assert result set
        self._assert_only_expected_resuls_are_in_actual_results(
            actual_result,
            queries.friends_older_than_me_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.friends_older_than_me_query)

    def test_how_many_countries_each_friend_visited(self):
        actual_result = self.redis_graph.query(queries.how_many_countries_each_friend_visited_query.query)

        # assert result set
        self._assert_only_expected_resuls_are_in_actual_results(
            actual_result,
            queries.how_many_countries_each_friend_visited_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.how_many_countries_each_friend_visited_query)

    def test_friends_age_statistics(self):
        actual_result = self.redis_graph.query(queries.friends_age_statistics_query.query)

        # assert result set
        self._assert_only_expected_resuls_are_in_actual_results(
            actual_result,
            queries.friends_age_statistics_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.friends_age_statistics_query)

    def test_visit_purpose_of_each_country_i_visited(self):
        actual_result = self.redis_graph.query(queries.visit_purpose_of_each_country_i_visited_query.query)

        # assert result set
        self._assert_only_expected_resuls_are_in_actual_results(
            actual_result,
            queries.visit_purpose_of_each_country_i_visited_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.visit_purpose_of_each_country_i_visited_query)

    def test_who_was_on_business_trip(self):
        actual_result = self.redis_graph.query(queries.who_was_on_business_trip_query.query)

        # assert result set
        self._assert_only_expected_resuls_are_in_actual_results(
            actual_result,
            queries.who_was_on_business_trip_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.who_was_on_business_trip_query)

    def test_number_of_vacations_per_person(self):
        NUM_EXPECTED_RESULTS = 6
        actual_result = self.redis_graph.query(queries.number_of_vacations_per_person_query.query)

        # assert result set
        self._assert_actual_results_contained_in_expected_results(
            actual_result,
            queries.number_of_vacations_per_person_query,
            NUM_EXPECTED_RESULTS)

        # assert query run time
        self._assert_run_time(actual_result, queries.number_of_vacations_per_person_query)


if __name__ == '__main__':
    unittest.main()
