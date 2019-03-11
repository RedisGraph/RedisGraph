import os
import sys
import unittest
from redisgraph import Graph

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/../../demo/social/')

# import redis
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
from disposableredis import DisposableRedis

from .reversepattern import ReversePattern
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

        # cls.r = redis.Redis()
        # redis_graph = Graph(social_utils.graph_name, cls.r)
        # social_utils.populate_graph(cls.r, redis_graph)

    @classmethod
    def tearDownClass(cls):
        cls.r.stop()
        # pass

    def assert_reversed_pattern(self, query, resultset):
        # Test reversed pattern query.
        reversed_query = ReversePattern().reverse_query_pattern(query)
        # print "reversed_query: %s" % reversed_query
        actual_result = redis_graph.query(reversed_query)

        # assert result set
        self.assertEqual(resultset.result_set, actual_result.result_set)

        # assert query run time
        self._assert_equalish(resultset.run_time_ms, actual_result.run_time_ms)
        
    def test00_graph_entities(self):
        global redis_graph
        q = queries.graph_entities.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.graph_entities)

        # assert query run time
        self._assert_run_time(actual_result, queries.graph_entities)
        
        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test01_relation_type_strings(self):
        global redis_graph
        q = queries.relation_type_counts.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.relation_type_counts)

        # assert query run time
        self._assert_run_time(actual_result, queries.relation_type_counts)

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test02_subset_of_people(self):
        global redis_graph
        q = queries.subset_of_people.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.subset_of_people)

        # assert query run time
        self._assert_run_time(actual_result, queries.subset_of_people)

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test03_my_friends(self):
        global redis_graph
        q = queries.my_friends_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.my_friends_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.my_friends_query)

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test04_friends_of_friends(self):
        global redis_graph
        q = queries.friends_of_friends_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.friends_of_friends_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.friends_of_friends_query)
        runtime = actual_result.run_time_ms

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test05_friends_of_friends_single_and_over_30(self):
        global redis_graph
        q = queries.friends_of_friends_single_and_over_30_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.friends_of_friends_single_and_over_30_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.friends_of_friends_single_and_over_30_query)

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test06_friends_of_friends_visited_amsterdam_and_single(self):
        global redis_graph
        q = queries.friends_of_friends_visited_amsterdam_and_single_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.friends_of_friends_visited_amsterdam_and_single_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.friends_of_friends_visited_amsterdam_and_single_query)

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test07_friends_visited_same_places_as_me(self):
        global redis_graph
        q = queries.friends_visited_same_places_as_me_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.friends_visited_same_places_as_me_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.friends_visited_same_places_as_me_query)

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test08_friends_older_than_me(self):
        global redis_graph
        q = queries.friends_older_than_me_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.friends_older_than_me_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.friends_older_than_me_query)

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test09_friends_age_difference_query(self):
        global redis_graph
        q = queries.friends_age_difference_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.friends_age_difference_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.friends_age_difference_query)

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test10_friends_who_are_older_than_average(self):
        global redis_graph
        q = queries.friends_who_are_older_than_average.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.friends_who_are_older_than_average)

        # assert query run time
        self._assert_run_time(actual_result, queries.friends_who_are_older_than_average)

    def test11_how_many_countries_each_friend_visited(self):
        global redis_graph
        q = queries.how_many_countries_each_friend_visited_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.how_many_countries_each_friend_visited_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.how_many_countries_each_friend_visited_query)

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test12_visit_purpose_of_each_country_i_visited(self):
        global redis_graph
        q = queries.visit_purpose_of_each_country_i_visited_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.visit_purpose_of_each_country_i_visited_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.visit_purpose_of_each_country_i_visited_query)
        
        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test13_who_was_on_business_trip(self):
        global redis_graph
        q = queries.who_was_on_business_trip_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.who_was_on_business_trip_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.who_was_on_business_trip_query)

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test14_number_of_vacations_per_person(self):
        global redis_graph
        NUM_EXPECTED_RESULTS = 6

        q = queries.number_of_vacations_per_person_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_actual_results_contained_in_expected_results(
            actual_result,
            queries.number_of_vacations_per_person_query,
            NUM_EXPECTED_RESULTS)

        # assert query run time
        self._assert_run_time(actual_result, queries.number_of_vacations_per_person_query)

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test15_all_reachable_friends_query(self):
        global redis_graph

        q = queries.all_reachable_friends_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.all_reachable_friends_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.all_reachable_friends_query)

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)
    
    def test16_all_reachable_countries_query(self):
        global redis_graph

        q = queries.all_reachable_countries_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.all_reachable_countries_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.all_reachable_countries_query)

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test17_reachable_countries_or_people_query(self):
        global redis_graph

        q = queries.reachable_countries_or_people_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.reachable_countries_or_people_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.reachable_countries_or_people_query)

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test18_all_reachable_countries_or_people_query(self):
        global redis_graph

        q = queries.all_reachable_countries_or_people_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.all_reachable_countries_or_people_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.all_reachable_countries_or_people_query)

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test19_all_reachable_entities_query(self):
        global redis_graph

        q = queries.all_reachable_entities_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.all_reachable_entities_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.all_reachable_entities_query)

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test20_happy_birthday(self):
        global redis_graph
        q = queries.happy_birthday_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.happy_birthday_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.happy_birthday_query)

    def test21_friends_age_statistics(self):
        global redis_graph
        q = queries.friends_age_statistics_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.friends_age_statistics_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.friends_age_statistics_query)
        
        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test22_delete_friendships(self):
        global redis_graph
        q = queries.delete_friendships_query.query
        actual_result = redis_graph.query(q)

        # assert query run time
        self._assert_run_time(actual_result, queries.delete_friendships_query)

    def test23_delete_person(self):
        global redis_graph
        q = queries.delete_person_query.query
        actual_result = redis_graph.query(q)

        # assert query run time
        self._assert_run_time(actual_result, queries.delete_person_query)

    def test24_post_delete_label(self):
        global redis_graph
        q = queries.post_delete_label_query.query
        actual_result = redis_graph.query(q)

         # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.post_delete_label_query)
        # assert query run time
        self._assert_run_time(actual_result, queries.post_delete_label_query)

if __name__ == '__main__':
    unittest.main()
