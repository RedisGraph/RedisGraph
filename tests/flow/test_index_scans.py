import os
import sys
import redis
import unittest
from redisgraph import Graph, Node, Edge
from base import FlowTestsBase

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/../../demo/social/')
import social_utils

redis_graph = None
dis_redis = None
redis_con = None

def get_redis():
    global dis_redis
    conn = redis.Redis()
    try:
        conn.ping()
        # Assuming RedisGraph is loaded.
    except redis.exceptions.ConnectionError:
        from .disposableredis import DisposableRedis
        # Bring up our own redis-server instance.
        dis_redis = DisposableRedis(loadmodule=os.path.dirname(os.path.abspath(__file__)) + '/../../src/redisgraph.so')
        dis_redis.start()
        conn = dis_redis.client()
    return conn

class IndexScanFlowTest(FlowTestsBase):

    @classmethod
    def setUpClass(cls):
        print "IndexScanFlowTest"
        global redis_graph
        global redis_con
        redis_con = get_redis()
        redis_graph = Graph(social_utils.graph_name, redis_con)
        social_utils.populate_graph(redis_con, redis_graph)
        cls.build_indices()

    @classmethod
    def tearDownClass(cls):
        if dis_redis is not None:
            dis_redis.stop()

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
        redis_graph.redis_con.execute_command("GRAPH.QUERY", "social", "CREATE INDEX ON :person(name)")
        query = "MATCH (a:person {name: 'Omri Traub'}), (b:person) WHERE b.age <= 30 RETURN a.name, b.name ORDER BY a.name, b.name"
	plan = redis_graph.execution_plan(query)
        # The two streams should both use index scans
        assert plan.count('Index Scan') == 2
        self.assertNotIn('Label Scan', plan)


        expected_result = [['a.name', 'b.name'],
                           ['Omri Traub', 'Gal Derriere'],
                           ['Omri Traub', 'Lucy Yanfital']]
        result = redis_graph.query(query)

        assert(result.result_set == expected_result)

if __name__ == '__main__':
    unittest.main()
