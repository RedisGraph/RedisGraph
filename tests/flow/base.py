import os
import warnings

class FlowTestsBase(object):

    def _assert_equalish(self, a, b, e=0.05):
        delta = a * e
        diff = abs(a-b)
        if diff > delta:
            warnings.warn('runtimes differ by more than \"%f\" percent' % e)
           
    def _assert_only_expected_results_are_in_actual_results(self,
                                                           actual_result,
                                                           query_info):
        actual_result_set = []
        if actual_result.result_set is not None:
            actual_result_set = actual_result.result_set

        # Assert number of results.
        self.env.assertEqual(len(actual_result_set), len(query_info.expected_result))

        # Assert actual values vs expected values.
        for res in query_info.expected_result:
            self.env.assertIn(res, actual_result_set)

    def _assert_actual_results_contained_in_expected_results(self,
                                                             actual_result,
                                                             query_info,
                                                             num_contained_results):
        actual_result_set = actual_result.result_set

        # Assert num results.
        self.env.assertEqual(len(actual_result_set), num_contained_results)

        # Assert actual values vs expected values.
        expected_result = query_info.expected_result
        count = len([res for res in expected_result if res in actual_result_set])

        # Assert number of different results is as expected.
        self.env.assertEqual(count, num_contained_results)


    def _assert_resultset_and_expected_mutually_included(self, actual_result, query_info):
        actual_result_set = []
        if actual_result.result_set is not None:
            actual_result_set = actual_result.result_set

        # Assert number of results.
        self.env.assertEqual(len(actual_result_set), len(query_info.expected_result))

        # Assert actual values vs expected values.
        for res in query_info.expected_result:
            self.env.assertIn(res, actual_result_set)
        
        # Assert expected values vs actual values.
        for res in actual_result_set:
            self.env.assertIn(res, query_info.expected_result)

    def _assert_resultset_equals_expected(self, actual_result, query_info):
        actual_result_set = actual_result.result_set or []
        self.env.assertEqual(actual_result_set, query_info.expected_result)

    def _assert_run_time(self, actual_result, query_info):
            if actual_result.run_time_ms > query_info.max_run_time_ms:
                warnings.warn('Maximum runtime for query \"%s\" was: %s, but should be %s' %
                             (query_info.description, str(actual_result.run_time_ms), str(query_info.max_run_time_ms)))
            
