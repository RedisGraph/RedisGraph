import os
import sys
from redisgraph import Graph, Node, Edge
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

    # Validate index utilization when filtering on a numeric field with the `IN` keyword.
    def test04_test_in_operator_numerics(self):
        # Validate the transformation of IN to multiple OR expressions.
        query = "MATCH (p:person) WHERE p.age IN [1,2,3] RETURN p"
        plan = redis_graph.execution_plan(query)
        self.env.assertIn('Index Scan', plan)

        # Validate that nested arrays are not scanned in index.
        query = "MATCH (p:person) WHERE p.age IN [[1,2],3] RETURN p"
        plan = redis_graph.execution_plan(query)
        self.env.assertNotIn('Index Scan', plan)
        self.env.assertIn('Label Scan', plan)

        # Validate the transformation of IN to multiple OR, over a range.
        query = "MATCH (p:person) WHERE p.age IN range(0,30) RETURN p.name ORDER BY p.name"
        plan = redis_graph.execution_plan(query)
        self.env.assertIn('Index Scan', plan)

        expected_result = [['Gal Derriere'], ['Lucy Yanfital']]
        result = redis_graph.query(query)
        self.env.assertEquals(result.result_set, expected_result)

         # Validate the transformation of IN to empty index iterator.
        query = "MATCH (p:person) WHERE p.age IN [] RETURN p.name"
        plan = redis_graph.execution_plan(query)
        self.env.assertIn('Index Scan', plan)

        expected_result = []
        result = redis_graph.query(query)
        self.env.assertEquals(result.result_set, expected_result)

        # Validate the transformation of IN OR IN to empty index iterators.
        query = "MATCH (p:person) WHERE p.age IN [] OR p.age IN [] RETURN p.name"
        plan = redis_graph.execution_plan(query)
        self.env.assertIn('Index Scan', plan)

        expected_result = []
        result = redis_graph.query(query)
        self.env.assertEquals(result.result_set, expected_result)

        # Validate the transformation of multiple IN filters.
        query = "MATCH (p:person) WHERE p.age IN [26, 27, 30] OR p.age IN [33, 34, 35] RETURN p.name ORDER BY p.age"
        plan = redis_graph.execution_plan(query)
        self.env.assertIn('Index Scan', plan)

        expected_result = [['Gal Derriere'], ['Lucy Yanfital'], ['Omri Traub'], ['Noam Nativ']]
        result = redis_graph.query(query)
        self.env.assertEquals(result.result_set, expected_result)

        # Validate the transformation of multiple IN filters.
        query = "MATCH (p:person) WHERE p.age IN [26, 27, 30] OR p.age IN [33, 34, 35] OR p.age IN [] RETURN p.name ORDER BY p.age"
        plan = redis_graph.execution_plan(query)
        self.env.assertIn('Index Scan', plan)

        expected_result = [['Gal Derriere'], ['Lucy Yanfital'], ['Omri Traub'], ['Noam Nativ']]
        result = redis_graph.query(query)
        self.env.assertEquals(result.result_set, expected_result)

    # Validate index utilization when filtering on string fields with the `IN` keyword.
    def test05_test_in_operator_string_props(self):
        # Build an index on the name property.
        redis_graph.redis_con.execute_command("GRAPH.QUERY", "social", "CREATE INDEX ON :person(name)")
        # Validate the transformation of IN to multiple OR expressions over string properties.
        query = "MATCH (p:person) WHERE p.name IN ['Gal Derriere', 'Lucy Yanfital'] RETURN p.name ORDER BY p.name"
        plan = redis_graph.execution_plan(query)
        self.env.assertIn('Index Scan', plan)
        self.env.assertNotIn('Label Scan', plan)

        expected_result = [['Gal Derriere'], ['Lucy Yanfital']]
        result = redis_graph.query(query)
        self.env.assertEquals(result.result_set, expected_result)

        # Combine numeric and string filters specified by IN.
        query = "MATCH (p:person) WHERE p.name IN ['Gal Derriere', 'Lucy Yanfital'] AND p.age in [30] RETURN p.name ORDER BY p.name"
        plan = redis_graph.execution_plan(query)
        self.env.assertIn('Index Scan', plan)
        self.env.assertNotIn('Label Scan', plan)

        expected_result = [['Lucy Yanfital']]
        result = redis_graph.query(query)
        self.env.assertEquals(result.result_set, expected_result)

         # Validate an empty index on IN with multiple indexes
        query = "MATCH (p:person) WHERE p.name IN [] OR p.age IN [] RETURN p.name"
        plan = redis_graph.execution_plan(query)
        self.env.assertIn('Index Scan', plan)

        expected_result = []
        result = redis_graph.query(query)
        self.env.assertEquals(result.result_set, expected_result)

        # Combine IN filters with other relational filters.
        query = "MATCH (p:person) WHERE p.name IN ['Gal Derriere', 'Lucy Yanfital'] AND p.name < 'H' RETURN p.name ORDER BY p.name"
        plan = redis_graph.execution_plan(query)
        self.env.assertIn('Index Scan', plan)
        self.env.assertNotIn('Label Scan', plan)

        expected_result = [['Gal Derriere']]
        result = redis_graph.query(query)
        self.env.assertEquals(result.result_set, expected_result)

        query = "MATCH (p:person) WHERE p.name IN ['Gal Derriere', 'Lucy Yanfital'] OR p.age = 33 RETURN p.name ORDER BY p.name"
        plan = redis_graph.execution_plan(query)
        self.env.assertIn('Index Scan', plan)
        self.env.assertNotIn('Label Scan', plan)

        expected_result = [['Gal Derriere'], ['Lucy Yanfital'], ['Omri Traub']]
        result = redis_graph.query(query)
        result = redis_graph.query(query)
        self.env.assertEquals(result.result_set, expected_result)

    # ',' is the default separator for tag indices
    # we've updated our separator to '\0' this test verifies issue 696:
    # https://github.com/RedisGraph/RedisGraph/issues/696
    def test06_tag_separator(self):
        redis_con = self.env.getConnection()
        redis_graph = Graph("G", redis_con)

        # Create a single node with a long string property, introduce a comma as part of the string.
        query = """CREATE (:Node{value:"A ValuePartition is a pattern that describes a restricted set of classes from which a property can be associated. The parent class is used in restrictions, and the covering axiom means that only members of the subclasses may be used as values."})"""
        redis_graph.query(query)

        # Index property.
        query = """CREATE INDEX ON :Node(value)"""
        redis_graph.query(query)

        # Make sure node is returned by index scan.
        query = """MATCH (a:Node{value:"A ValuePartition is a pattern that describes a restricted set of classes from which a property can be associated. The parent class is used in restrictions, and the covering axiom means that only members of the subclasses may be used as values."}) RETURN a"""
        plan = redis_graph.execution_plan(query)
        result_set = redis_graph.query(query).result_set
        self.env.assertIn('Index Scan', plan)
        self.env.assertEqual(len(result_set), 1)
