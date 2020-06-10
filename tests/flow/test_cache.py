from RLTest import Env
from redisgraph import Graph, Node, Edge

from base import FlowTestsBase

redis_con = None

class testCache(FlowTestsBase):

    def __init__(self):
        # Have only one thread handling queries
        self.env = Env(moduleArgs='THREAD_COUNT 1 CACHE_SIZE 3')
        global redis_con
        redis_con = self.env.getConnection()

    def compare_uncached_to_cached_query(self, query, params = None):
        global redis_con
        plan_graph = Graph('Cache_Test_plans', redis_con)
        uncached_plan = plan_graph.execution_plan(query)
        cached_plan = plan_graph.execution_plan(query)
        self.env.assertEqual(uncached_plan, cached_plan)
        # Delete the graph to flush the cache
        plan_graph.delete()
        graph = Graph('Cache_Test_results', redis_con)
        uncached_result = graph.query(query, params)
        self.env.assertFalse(uncached_result.cached_execution)
        cached_result = graph.query(query, params)
        self.env.assertTrue(cached_result.cached_execution)
        self.env.assertEqual(uncached_result.result_set, cached_result.result_set)
        graph.delete()

    def sanity_check(self):
        graph = Graph('Cacche_Sanity_Check')

    def test01_testCreate(self):
        graph = Graph('Cache_Test_Create', redis_con)
        query = "CREATE ()"
        uncached_result = graph.query(query)
        self.env.assertFalse(uncached_result.cached_execution)
        cached_result = graph.query(query)
        self.env.assertTrue(cached_result.cached_execution)
        self.env.assertEqual(uncached_result.result_set, cached_result.result_set)
        self.env.assertEqual(uncached_result.nodes_created, cac)
        graph.delete()
        

    # def test01_test_simple_scalar(self):
        # graph = Graph('Cache_Test_results', redis_con)
        # # graph.query(            """
        # #     CREATE (a {name: 'David'}),
        # #     (b {name: 'Other'}),
        # #     (c {name: 'NotOther'}),
        # #     (d {name: 'NotOther2'}),
        # #     (a)-[:REL]->(b),
        # #     (a)-[:REL]->(c),
        # #     (a)-[:REL]->(d),
        # #     (b)-[:REL]->(),
        # #     (b)-[:REL]->(),
        # #     (c)-[:REL]->(),
        # #     (c)-[:REL]->(),
        # #     (d)-[:REL]->()
        # #     """)
        # query = """
        #     MATCH (david {name: 'David'})--(otherPerson)-->()
        #     WITH otherPerson, count(*) AS foaf
        #     WHERE foaf > 1
        #     WITH otherPerson
        #     WHERE otherPerson.name <> 'NotOther'
        #     RETURN count(*)
        #     """
        # self.compare_uncached_to_cached_query(query)
        # # query = "RETURN 1.2"
        # # self.compare_uncached_to_cached_query(query)
        # # query = "RETURN 'str''"
        # # self.compare_uncached_to_cached_query(query)
        # # query = "RETURN true"
        # # self.compare_uncached_to_cached_query(query)
        # # query = "RETURN false"
        # # self.compare_uncached_to_cached_query(query)
        # # query = "RETURN null"
        # # self.compare_uncached_to_cached_query(query)