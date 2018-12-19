import os
import sys
import unittest
from redisgraph import Graph, Node, Edge
from .disposableredis import DisposableRedis
from base import FlowTestsBase

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/../../demo/social/')
import social_utils

redis_graph = None

def redis():
    return DisposableRedis(loadmodule=os.path.dirname(os.path.abspath(__file__)) + '/../../src/redisgraph.so')

class IndexScanFlowTest(FlowTestsBase):

    @classmethod
    def setUpClass(cls):
        print "IndexScanFlowTest"
        global redis_graph
        cls.r = redis()
        cls.r.start()
        redis_con = cls.r.client()
        redis_graph = Graph(social_utils.graph_name, redis_con)
        social_utils.populate_graph(redis_con, redis_graph)
        cls.build_indices()

    @classmethod
    def tearDownClass(cls):
        cls.r.stop()

    @classmethod
    def build_indices(self):
        redis_graph.redis_con.execute_command("GRAPH.QUERY", "social", "CREATE INDEX ON :person(age)")
	redis_graph.redis_con.execute_command("GRAPH.QUERY", "social", "CREATE INDEX ON :country(name)")

    # Validate that Cartesian products using index and label scans succeed
    def test01_cartesian_product_mixed_scans(self):
        query = "MATCH (p:person), (c:country) WHERE p.age > 0 RETURN p.age, c.name ORDER BY p.age, c.name"
	plan = redis_graph.execution_plan(query)
        self.assertIn('Index Scan', plan)
        self.assertIn('Label Scan', plan)
        indexed_result = redis_graph.query(query)

        query = "MATCH (p:person), (c:country) RETURN p.age, c.name ORDER BY p.age, c.name"
	plan = redis_graph.execution_plan(query)
        self.assertNotIn('Index Scan', plan)
        self.assertIn('Label Scan', plan)
        unindexed_result = redis_graph.query(query)

	assert(indexed_result.result_set == unindexed_result.result_set)

    # Validate that Cartesian products using just index scans succeed
    def test02_cartesian_product_index_scans_only(self):
        query = "MATCH (p:person), (c:country) WHERE p.age > 0 AND c.name > '' RETURN p.age, c.name ORDER BY p.age, c.name"
	plan = redis_graph.execution_plan(query)
        # The two streams should both use index scans
        assert plan.count('Index Scan') == 2
        self.assertNotIn('Label Scan', plan)
        indexed_result = redis_graph.query(query)

        query = "MATCH (p:person), (c:country) RETURN p.age, c.name ORDER BY p.age, c.name"
	plan = redis_graph.execution_plan(query)
        self.assertNotIn('Index Scan', plan)
        self.assertIn('Label Scan', plan)
        unindexed_result = redis_graph.query(query)

	assert(indexed_result.result_set == unindexed_result.result_set)

    # Validate that the appropriate bounds are respected when a Cartesian product uses the same index in two streams
    def test03_cartesian_product_reused_index(self):
        query = "MATCH (a:country {name: 'Japan'}), (b:country {name: 'Germany'}) RETURN a, b"
	plan = redis_graph.execution_plan(query)
        # The two streams should both use index scans
        assert plan.count('Index Scan') == 2
        self.assertNotIn('Label Scan', plan)

        expected_result = [['a.name', 'b.name'],
                           ['Japan', 'Germany']]
        result = redis_graph.query(query)

        assert(result.result_set == expected_result)

if __name__ == '__main__':
    unittest.main()
