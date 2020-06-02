import os
import sys
from RLTest import Env
from redisgraph import Graph

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/../../demo/social/')
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from reversepattern import ReversePattern
from base import FlowTestsBase
import social_queries as queries
import social_utils

redis_graph = None

class testSocialFlow(FlowTestsBase):

    def __init__(self):
        self.env = Env()
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(social_utils.graph_name, redis_con)
        social_utils.populate_graph(redis_con, redis_graph)
 
    def assert_reversed_pattern(self, query, resultset):
        # Test reversed pattern query.
        reversed_query = ReversePattern().reverse_query_pattern(query)
        # print "reversed_query: %s" % reversed_query
        actual_result = redis_graph.query(reversed_query)

        # assert result set
        self.env.assertEqual(resultset.result_set, actual_result.result_set)

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

    def test06_friends_of_friends_visited_netherlands_and_single(self):
        global redis_graph
        q = queries.friends_of_friends_visited_netherlands_and_single_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.friends_of_friends_visited_netherlands_and_single_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.friends_of_friends_visited_netherlands_and_single_query)

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

    def test08_countries_visited_by_roi_tal_boaz(self):
        global redis_graph
        q = queries.countries_visited_by_roi_tal_boaz.query
        actual_result = redis_graph.query(q)
        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.countries_visited_by_roi_tal_boaz)

        # assert query run time
        self._assert_run_time(actual_result, queries.countries_visited_by_roi_tal_boaz)
        
    def test09_friends_older_than_me(self):
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

    def test10_friends_age_difference_query(self):
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

    def test11_friends_who_are_older_than_average(self):
        global redis_graph
        q = queries.friends_who_are_older_than_average.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.friends_who_are_older_than_average)

        # assert query run time
        self._assert_run_time(actual_result, queries.friends_who_are_older_than_average)

    def test12_how_many_countries_each_friend_visited(self):
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

    def test13_visit_purpose_of_each_country_i_visited(self):
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

    def test14_who_was_on_business_trip(self):
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

    def test15_number_of_vacations_per_person(self):
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

    def test16_all_reachable_friends_query(self):
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
    
    def test17_all_reachable_countries_query(self):
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

    def test18_reachable_countries_or_people_query(self):
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

    def test19_all_reachable_countries_or_people_query(self):
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

    def test20_all_reachable_entities_query(self):
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
    
    def test21_all_reachable_people_min_2_hops_query(self):
        global redis_graph

        q = queries.all_reachable_people_min_2_hops_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.all_reachable_people_min_2_hops_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.all_reachable_people_min_2_hops_query)

        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test22_happy_birthday(self):
        global redis_graph
        q = queries.happy_birthday_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.happy_birthday_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.happy_birthday_query)

    def test23_friends_age_statistics(self):
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
    
    def test24_all_paths_leads_to_greece_query(self):
        global redis_graph
        q = queries.all_paths_leads_to_greece_query.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.all_paths_leads_to_greece_query)

        # assert query run time
        self._assert_run_time(actual_result, queries.all_paths_leads_to_greece_query)
        
        # assert reversed pattern.
        self.assert_reversed_pattern(q, actual_result)

    def test25_number_of_paths_to_places_visited(self):
        global redis_graph
        q = queries.number_of_paths_to_places_visited.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.number_of_paths_to_places_visited)

        # assert query run time
        self._assert_run_time(actual_result, queries.number_of_paths_to_places_visited)

    def test26_pagerank_friends(self):
        global redis_graph
        q = queries.pagerank_friends.query
        actual_result = redis_graph.query(q)

        # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.pagerank_friends)

        # assert query run time
        self._assert_run_time(actual_result, queries.pagerank_friends)

    def test27_edge_counting(self):
        global redis_graph
        aggregations = [
            "match (a:person)-[e]->(b) return a.name, count(e) ORDER BY a.name",    # Number of outgoing edges.
            "match (a)-[e]->(b:person) return b.name, count(e) ORDER BY b.name",    # Number of incoming edges.
            "match (a)-[e:friend]->(b) return a.name, count(e) ORDER BY a.name",    # Number of typed outgoing edges.
            "match (a)-[e:friend]->(b) return b.name, count(e) ORDER BY b.name"     # Number of typed incoming edges.
        ]

        none_aggregation = [
            "match (a:person) WHERE outdegree(a) > 0 RETURN a.name, outdegree(a) ORDER BY a.name",  # Number of outgoing edges.
            "match (a:person) WHERE indegree(a) > 0 RETURN a.name, indegree(a) ORDER BY a.name",    # Number of incoming edges.
            "match (a:person) WHERE outdegree(a, 'friend') > 0 RETURN a.name, outdegree(a, 'friend') ORDER BY a.name",  # Number of typed outgoing edges.
            "match (a:person) WHERE indegree(a, 'friend') > 0 RETURN a.name, indegree(a, 'friend') ORDER BY a.name"     # Number of typed incoming edges.
        ]

        for i in range(len(aggregations)):
            result_agg = redis_graph.query(aggregations[i]).result_set
            result_none_agg = redis_graph.query(none_aggregation[i]).result_set
            self.env.assertTrue(result_agg == result_none_agg)

    def test28_delete_friendships(self):
        global redis_graph
        q = queries.delete_friendships_query.query
        actual_result = redis_graph.query(q)

        # assert query run time
        self._assert_run_time(actual_result, queries.delete_friendships_query)

    def test29_delete_person(self):
        global redis_graph
        q = queries.delete_person_query.query
        actual_result = redis_graph.query(q)

        # assert query run time
        self._assert_run_time(actual_result, queries.delete_person_query)

    def test30_post_delete_label(self):
        global redis_graph
        q = queries.post_delete_label_query.query
        actual_result = redis_graph.query(q)

         # assert result set
        self._assert_only_expected_results_are_in_actual_results(
            actual_result,
            queries.post_delete_label_query)
        # assert query run time
        self._assert_run_time(actual_result, queries.post_delete_label_query)
