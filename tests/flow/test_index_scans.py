import os
import sys
from redisgraph import Graph, Node, Edge

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
from base import FlowTestsBase

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/../../demo/social/')
import social_utils

redis_graph = None

class testIndexScanFlow(FlowTestsBase):
    def __init__(self):
        super(testIndexScanFlow, self).__init__()        

    def setUp(self):
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(social_utils.graph_name, redis_con)
        social_utils.populate_graph(redis_con, redis_graph)
        self.build_indices()

    def tearDown(self):
        self.env.cmd('flushall')

    def build_indices(self):
        global redis_graph
        redis_graph.redis_con.execute_command("GRAPH.QUERY", "social", "CREATE INDEX ON :person(age)")
        redis_graph.redis_con.execute_command("GRAPH.QUERY", "social", "CREATE INDEX ON :country(name)")

    # Validate that Cartesian products using index and label scans succeed
    def test01_cartesian_product_mixed_scans(self):
        query = "MATCH (p:person), (c:country) WHERE p.age > 0 RETURN p.age, c.name ORDER BY p.age, c.name"
        plan = redis_graph.execution_plan(query)
        self.env.assertIn('Index Scan', plan)
        self.env.assertIn('Label Scan', plan)
        indexed_result = redis_graph.query(query)

        query = "MATCH (p:person), (c:country) RETURN p.age, c.name ORDER BY p.age, c.name"
        plan = redis_graph.execution_plan(query)
        self.env.assertNotIn('Index Scan', plan)
        self.env.assertIn('Label Scan', plan)
        unindexed_result = redis_graph.query(query)

        self.env.assertEquals(indexed_result.result_set, unindexed_result.result_set)

    # Validate that Cartesian products using just index scans succeed
    def test02_cartesian_product_index_scans_only(self):
        query = "MATCH (p:person), (c:country) WHERE p.age > 0 AND c.name > '' RETURN p.age, c.name ORDER BY p.age, c.name"
        plan = redis_graph.execution_plan(query)
        # The two streams should both use index scans
        self.env.assertEquals(plan.count('Index Scan'), 2)
        self.env.assertNotIn('Label Scan', plan)
        indexed_result = redis_graph.query(query)

        query = "MATCH (p:person), (c:country) RETURN p.age, c.name ORDER BY p.age, c.name"
        plan = redis_graph.execution_plan(query)
        self.env.assertNotIn('Index Scan', plan)
        self.env.assertIn('Label Scan', plan)
        unindexed_result = redis_graph.query(query)

        self.env.assertEquals(indexed_result.result_set, unindexed_result.result_set)

    # Validate that the appropriate bounds are respected when a Cartesian product uses the same index in two streams
    def test03_cartesian_product_reused_index(self):
        redis_graph.redis_con.execute_command("GRAPH.QUERY", "social", "CREATE INDEX ON :person(name)")
        query = "MATCH (a:person {name: 'Omri Traub'}), (b:person) WHERE b.age <= 30 RETURN a.name, b.name ORDER BY a.name, b.name"
        plan = redis_graph.execution_plan(query)
        # The two streams should both use index scans
        self.env.assertEquals(plan.count('Index Scan'), 2)
        self.env.assertNotIn('Label Scan', plan)


        expected_result = [['Omri Traub', 'Gal Derriere'],
                           ['Omri Traub', 'Lucy Yanfital']]
        result = redis_graph.query(query)

        self.env.assertEquals(result.result_set, expected_result)
