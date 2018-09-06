import os
import sys
import unittest
from redisgraph import Graph

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/../../demo/social/')
from .disposableredis import DisposableRedis
from base import FlowTestsBase
import social_queries as queries
import social_utils

redis_graph = None

def redis():
    return DisposableRedis(loadmodule=os.path.dirname(os.path.abspath(__file__)) + '/../../src/redisgraph.so')

class SocialFlowTest(FlowTestsBase):
    @classmethod
    def setUpClass(cls):
        print "SocialFlowTest"
        global redis_graph
        cls.r = redis()
        cls.r.start()
        redis_con = cls.r.client()
        redis_graph = Graph(social_utils.graph_name, redis_con)
        social_utils.populate_graph(redis_con, redis_graph)

    @classmethod
    def tearDownClass(cls):
        cls.r.stop()
    
    def test00_subset_of_people(self):
        global redis_graph
        actual_result = redis_graph.query(queries.subset_of_people.query)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.subset_of_people)

        # assert query run time
        self._assert_run_time(actual_result, queries.subset_of_people)

    def test01_my_friends(self):
        global redis_graph
        actual_result = redis_graph.query(queries.my_friends_query.query)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.my_friends_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.my_friends_query)

    def test02_friends_of_friends(self):
        global redis_graph
        actual_result = redis_graph.query(queries.friends_of_friends_query.query)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.friends_of_friends_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.friends_of_friends_query)

    def test03_friends_of_friends_single_and_over_30(self):
        global redis_graph
        actual_result = redis_graph.query(queries.friends_of_friends_single_and_over_30_query.query)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.friends_of_friends_single_and_over_30_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.friends_of_friends_single_and_over_30_query)

    def test04_friends_of_friends_visited_amsterdam_and_single(self):
        global redis_graph
        actual_result = redis_graph.query(queries.friends_of_friends_visited_amsterdam_and_single_query.query)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.friends_of_friends_visited_amsterdam_and_single_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.friends_of_friends_visited_amsterdam_and_single_query)

    def test05_friends_visited_same_places_as_me(self):
        global redis_graph
        actual_result = redis_graph.query(queries.friends_visited_same_places_as_me_query.query)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.friends_visited_same_places_as_me_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.friends_visited_same_places_as_me_query)

    def test06_friends_older_than_me(self):
        global redis_graph
        actual_result = redis_graph.query(queries.friends_older_than_me_query.query)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.friends_older_than_me_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.friends_older_than_me_query)

    def test07_how_many_countries_each_friend_visited(self):
        global redis_graph
        actual_result = redis_graph.query(queries.how_many_countries_each_friend_visited_query.query)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.how_many_countries_each_friend_visited_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.how_many_countries_each_friend_visited_query)

    def test08_happy_birthday(self):
        global redis_graph
        actual_result = redis_graph.query(queries.happy_birthday_query.query)

        # assert query run time
        self._assert_run_time(actual_result, queries.happy_birthday_query)

    def test09_friends_age_statistics(self):
        global redis_graph
        actual_result = redis_graph.query(queries.friends_age_statistics_query.query)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.friends_age_statistics_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.friends_age_statistics_query)

    def test10_visit_purpose_of_each_country_i_visited(self):
        global redis_graph
        actual_result = redis_graph.query(queries.visit_purpose_of_each_country_i_visited_query.query)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.visit_purpose_of_each_country_i_visited_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.visit_purpose_of_each_country_i_visited_query)

    def test11_who_was_on_business_trip(self):
        global redis_graph
        actual_result = redis_graph.query(queries.who_was_on_business_trip_query.query)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.who_was_on_business_trip_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.who_was_on_business_trip_query)

    def test12_number_of_vacations_per_person(self):
        global redis_graph
        NUM_EXPECTED_RESULTS = 6

        actual_result = redis_graph.query(queries.number_of_vacations_per_person_query.query)

        # assert result set
        self._assert_actual_results_contained_in_expected_results(
            actual_result,
            queries.number_of_vacations_per_person_query,
            NUM_EXPECTED_RESULTS)

        # assert query run time
        self._assert_run_time(actual_result, queries.number_of_vacations_per_person_query)

    def test13_delete_friendships(self):
        global redis_graph
        actual_result = redis_graph.query(queries.delete_friendships_query.query)

        # assert query run time
        self._assert_run_time(actual_result, queries.delete_friendships_query)

    def test14_delete_person(self):
        global redis_graph
        actual_result = redis_graph.query(queries.delete_person_query.query)

        # assert query run time
        self._assert_run_time(actual_result, queries.delete_person_query)

    def test_15_post_delete_label(self):
        global redis_graph
        actual_result = redis_graph.query(queries.post_delete_label_query.query)

         # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.post_delete_label_query)
        # assert query run time
        self._assert_run_time(actual_result, queries.post_delete_label_query)

if __name__ == '__main__':
    unittest.main()
