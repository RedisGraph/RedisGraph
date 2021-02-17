from RLTest import Env
from redisgraph import Graph, Node, Edge

from base import FlowTestsBase

redis_con = None

CACHE_SIZE = 16

class testCache(FlowTestsBase):

    def __init__(self):
        # Have only one thread handling queries
        self.env = Env(decodeResponses=True, moduleArgs='THREAD_COUNT 8 CACHE_SIZE {CACHE_SIZE}'.format(CACHE_SIZE = CACHE_SIZE))
        global redis_con
        redis_con = self.env.getConnection()

    def compare_uncached_to_cached_query_plans(self, query):
        global redis_con
        plan_graph = Graph('Cache_Test_plans', redis_con)
        uncached_plan = plan_graph.execution_plan(query)
        cached_plan = plan_graph.execution_plan(query)
        self.env.assertEqual(uncached_plan, cached_plan)
        plan_graph.delete()

    def test_sanity_check(self):
        graph = Graph('Cache_Sanity_Check', redis_con)
        for i in range(CACHE_SIZE + 1):
            result = graph.query("MATCH (n) WHERE n.value = {val} RETURN n".format(val=i))
            self.env.assertFalse(result.cached_execution)
        
        for i in range(1,CACHE_SIZE + 1):
            result = graph.query("MATCH (n) WHERE n.value = {val} RETURN n".format(val=i))
            self.env.assertTrue(result.cached_execution)
        
        result = graph.query("MATCH (n) WHERE n.value = 0 RETURN n")
        self.env.assertFalse(result.cached_execution)

        graph.delete()

    def test01_test_create(self):
        # Both queries do exactly the same operations
        graph = Graph('Cache_Test_Create', redis_con)
        query = "CREATE ()"
        self.compare_uncached_to_cached_query_plans(query)
        uncached_result = graph.query(query)
        cached_result = graph.query(query)
        self.env.assertFalse(uncached_result.cached_execution)
        self.env.assertTrue(cached_result.cached_execution)
        self.env.assertEqual(uncached_result.nodes_created, cached_result.nodes_created)
        graph.delete()
        
    def test02_test_create_with_params(self):
        # Both queries do exactly the same operations
        graph = Graph('Cache_Test_Create_With_Params', redis_con)
        params = {'val' : 1}
        query = "CREATE ({val:$val})"
        self.compare_uncached_to_cached_query_plans(query)
        uncached_result = graph.query(query, params)
        params = {'val' : 2}
        cached_result = graph.query(query, params)
        self.env.assertFalse(uncached_result.cached_execution)
        self.env.assertTrue(cached_result.cached_execution)
        self.env.assertEqual(uncached_result.nodes_created, cached_result.nodes_created)
        graph.delete()

    def test03_test_delete(self):
        # Both queries do exactly the same operations
        graph = Graph('Cache_Test_Delete', redis_con)
        for i in range(2):
            params = {'val' : i}
            query = "CREATE ({val:$val})-[:R]->()"
            graph.query(query, params)
        
        params = {'val': 0}
        query = "MATCH (n {val:$val}) DELETE n"
        self.compare_uncached_to_cached_query_plans(query)
        uncached_result = graph.query(query, params)
        params = {'val': 1}
        cached_result = graph.query(query, params)
        self.env.assertFalse(uncached_result.cached_execution)
        self.env.assertTrue(cached_result.cached_execution)
        self.env.assertEqual(uncached_result.relationships_deleted, cached_result.relationships_deleted)
        self.env.assertEqual(uncached_result.nodes_deleted, cached_result.nodes_deleted)
        graph.delete()

    def test04_test_merge(self):
        # Different outcome, same execution plan.
        graph = Graph('Cache_Test_Merge', redis_con)    
        params = {'create_val': 0, 'match_val':1}
        query = "MERGE (n) ON CREATE SET n.val = $create_val ON MATCH SET n.val = $match_val RETURN n.val"
        self.compare_uncached_to_cached_query_plans(query)
        uncached_result = graph.query(query, params)
        cached_result = graph.query(query, params)
        self.env.assertFalse(uncached_result.cached_execution)
        self.env.assertTrue(cached_result.cached_execution)
        self.env.assertEqual(uncached_result.properties_set, cached_result.properties_set)
        self.env.assertEqual([[0]], uncached_result.result_set)
        self.env.assertEqual(1, uncached_result.nodes_created)
        self.env.assertEqual([[1]], cached_result.result_set)
        self.env.assertEqual(0, cached_result.nodes_created)

        graph.delete()

    def test05_test_branching_with_path_filter(self):
        # Different outcome, same execution plan.
        graph = Graph('Cache_Test_Path_Filter', redis_con) 
        query = "CREATE ({val:1})-[:R]->({val:2})-[:R2]->({val:3})"
        graph.query(query)
        query = "MATCH (n) WHERE (n)-[:R]->({val:$val}) OR (n)-[:R2]->({val:$val}) RETURN n.val"
        self.compare_uncached_to_cached_query_plans(query)
        params = {'val':2}
        uncached_result = graph.query(query, params)
        params = {'val':3}
        cached_result = graph.query(query, params)
        self.env.assertFalse(uncached_result.cached_execution)
        self.env.assertTrue(cached_result.cached_execution)
        self.env.assertEqual([[1]], uncached_result.result_set)
        self.env.assertEqual([[2]], cached_result.result_set)
        graph.delete()


    def test06_test_optimizations_index(self):
        graph = Graph('Cache_Test_Index', redis_con)
        graph.query("CREATE INDEX ON :N(val)")
        query = "CREATE (:N{val:1}), (:N{val:2})"
        graph.query(query)
        query = "MATCH (n:N{val:$val}) RETURN n.val"
        self.compare_uncached_to_cached_query_plans(query)
        params = {'val':1}
        uncached_result = graph.query(query, params)
        params = {'val':2}
        cached_result = graph.query(query, params)
        self.env.assertFalse(uncached_result.cached_execution)
        self.env.assertTrue(cached_result.cached_execution)
        self.env.assertEqual([[1]], uncached_result.result_set)
        self.env.assertEqual([[2]], cached_result.result_set)
        graph.delete()


    def test07_test_optimizations_id_scan(self):
        graph = Graph('Cache_Test_ID_Scan', redis_con)
        query = "CREATE (), ()"
        graph.query(query)
        query = "MATCH (n) WHERE ID(n)=$id RETURN id(n)"
        self.compare_uncached_to_cached_query_plans(query)
        params = {'id':0}
        uncached_result = graph.query(query, params)
        params = {'id':1}
        cached_result = graph.query(query, params)
        self.env.assertFalse(uncached_result.cached_execution)
        self.env.assertTrue(cached_result.cached_execution)
        self.env.assertEqual([[0]], uncached_result.result_set)
        self.env.assertEqual([[1]], cached_result.result_set)
        graph.delete()


    def test08_test_join(self):
        graph = Graph('Cache_Test_Join', redis_con)
        query = "CREATE ({val:1}), ({val:2}), ({val:3}),({val:4})"
        graph.query(query)
        query = "MATCH (a {val:$val}), (b) WHERE a.val = b.val-1 RETURN a.val, b.val "
        self.compare_uncached_to_cached_query_plans(query)
        params = {'val':1}
        uncached_result = graph.query(query, params)
        params = {'val':3}
        cached_result = graph.query(query, params)
        self.env.assertFalse(uncached_result.cached_execution)
        self.env.assertTrue(cached_result.cached_execution)
        self.env.assertEqual([[1, 2]], uncached_result.result_set)
        self.env.assertEqual([[3, 4]], cached_result.result_set)
        graph.delete()

    def test09_test_edge_merge(self):
        # In this scenario, the same query is executed twice.
        # In the first time, the relationship `leads` is unknown to the graph so it is created.
        # In the second time the relationship should be known to the graph, so it will be returned by the match.
        # The test validates that a valid edge is returned.
        graph = Graph('Cache_Test_Edge_Merge', redis_con)
        query = "CREATE ({val:1}), ({val:2})"
        graph.query(query)
        query = "MATCH (a {val:1}), (b {val:2}) MERGE (a)-[e:leads]->(b) RETURN e"
        self.compare_uncached_to_cached_query_plans(query)
        uncached_result = graph.query(query)
        self.env.assertEqual(1, uncached_result.relationships_created)
        cached_result = graph.query(query)
        self.env.assertEqual(0, cached_result.relationships_created)
        self.env.assertEqual(uncached_result.result_set, cached_result.result_set)

    def test10_test_labelscan_update(self):
        # In this scenario a label scan is made for non existing label
        # than the label is created and the label scan query is re-used.
        graph = Graph('Cache_test_labelscan_update', redis_con)
        query = "MATCH (n:Label) return n"
        result = graph.query(query)
        self.env.assertEqual(0, len(result.result_set))
        query = "MERGE (n:Label)"
        result = graph.query(query)
        self.env.assertEqual(1, result.nodes_created)
        query = "MATCH (n:Label) return n"
        result = graph.query(query)
        self.env.assertEqual(1, len(result.result_set))
        self.env.assertEqual("Label", result.result_set[0][0].label)

    def test11_test_index_scan_update(self):
        # In this scenario a label scan and Update op are made for non-existent label,
        # then the label is created and an index are subsequently created.
        # When the cached query is reused, it should rely on valid label data.
        graph = Graph('Cache_test_index_scan_update', redis_con)
        params = {'v': 1}
        query = "MERGE (n:Label {v: 1}) SET n.v = $v"
        result = graph.query(query, params)
        self.env.assertEqual(0, len(result.result_set))
        self.env.assertEqual(1, result.nodes_created)
        self.env.assertEqual(1, result.labels_added)

        query = "CREATE INDEX ON :Label(v)"
        result = graph.query(query)
        self.env.assertEqual(1, result.indices_created)

        params = {'v': 5}
        query = "MERGE (n:Label {v: 1}) SET n.v = $v"
        result = graph.query(query, params)
        self.env.assertEqual(0, result.nodes_created)
        self.env.assertEqual(1, result.properties_set)

    def test12_test_skip_limit(self):
        # Test using parameters for skip and limit values,
        # ensuring cached executions always use the parameterized values.
        graph = Graph('Cache_Empty_Key', redis_con)
        query = "UNWIND [1,2,3,4] AS arr RETURN arr SKIP $s LIMIT $l"
        params = {'s': 1, 'l': 1}
        uncached_result = graph.query(query, params)
        expected_result = [[2]]
        cached_result = graph.query(query, params)
        self.env.assertEqual(expected_result, cached_result.result_set)
        self.env.assertEqual(uncached_result.result_set, cached_result.result_set)
        self.env.assertFalse(uncached_result.cached_execution)
        self.env.assertTrue(cached_result.cached_execution)

        # Update the params
        params = {'s': 2, 'l': 2}
        # The new result should respect the new skip and limit.
        expected_result = [[3], [4]]
        cached_result = graph.query(query, params)
        self.env.assertEqual(expected_result, cached_result.result_set)
        self.env.assertTrue(cached_result.cached_execution)
