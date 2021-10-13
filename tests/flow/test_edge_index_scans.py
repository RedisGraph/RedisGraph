import os
import sys
from RLTest import Env
from redisgraph import Graph, Node, Edge
from base import FlowTestsBase

people = ["Roi", "Alon", "Ailon", "Boaz", "Tal", "Omri", "Ori"]
redis_graph = None

class testEdgeByIndexScanFlow(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)

    def setUp(self):
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph("social", redis_con)
        self.populate_graph(redis_graph)
        self.build_indices()

    def tearDown(self):
        self.env.cmd('flushall')
    
    def populate_graph(self, redis_graph):
        nodes = {}

        # Create entities
        for p in people:
            node = Node(label="person", properties={"name": p})
            redis_graph.add_node(node)
            nodes[p] = node

        # Fully connected graph
        edge_id = 0
        for src in nodes:
            for dest in nodes:
                if src != dest:
                    edge = Edge(nodes[src], "knows", nodes[dest], properties={"created_at": edge_id * 2})
                    redis_graph.add_edge(edge)
                    edge = Edge(nodes[src], "friend", nodes[dest], properties={"created_at": edge_id * 2 + 1, "updated_at": edge_id * 3})
                    redis_graph.add_edge(edge)
                    edge_id = edge_id + 1

        redis_graph.commit()

    def build_indices(self):
        global redis_graph
        redis_graph.query("CREATE INDEX ON :person(age)")
        redis_graph.query("CREATE INDEX FOR ()-[f:friend]-() ON (f.created_at)")
        redis_graph.query("CREATE INDEX FOR ()-[f:knows]-() ON (f.created_at)")

    # Validate that Cartesian products using index and label scans succeed
    def test01_cartesian_product_mixed_scans(self):
        query = "MATCH ()-[f:friend]->(), ()-[k:knows]->() WHERE f.created_at >= 0 RETURN f.created_at, k.created_at ORDER BY f.created_at, k.created_at"
        plan = redis_graph.execution_plan(query)
        self.env.assertIn('Edge By Index Scan', plan)
        self.env.assertIn('Conditional Traverse', plan)
        indexed_result = redis_graph.query(query)

        query = "MATCH ()-[f:friend]->(), ()-[k:knows]->() RETURN f.created_at, k.created_at ORDER BY f.created_at, k.created_at"
        plan = redis_graph.execution_plan(query)
        self.env.assertNotIn('Edge By Index Scan', plan)
        self.env.assertIn('Conditional Traverse', plan)
        unindexed_result = redis_graph.query(query)

        self.env.assertEquals(indexed_result.result_set, unindexed_result.result_set)

    # Validate that Cartesian products using just index scans succeed
    def test02_cartesian_product_index_scans_only(self):
        query = "MATCH ()-[f:friend]->(), ()-[k:knows]->() WHERE f.created_at >= 0 AND k.created_at >= 0 RETURN f.created_at, k.created_at ORDER BY f.created_at, k.created_at"
        plan = redis_graph.execution_plan(query)
        # The two streams should both use index scans
        self.env.assertEquals(plan.count('Edge By Index Scan'), 2)
        self.env.assertNotIn('Conditional Traverse', plan)
        indexed_result = redis_graph.query(query)

        query = "MATCH ()-[f:friend]->(), ()-[k:knows]->() RETURN f.created_at, k.created_at ORDER BY f.created_at, k.created_at"
        plan = redis_graph.execution_plan(query)
        self.env.assertNotIn('Edge By Index Scan', plan)
        self.env.assertIn('Conditional Traverse', plan)
        unindexed_result = redis_graph.query(query)

        self.env.assertEquals(indexed_result.result_set, unindexed_result.result_set)

    # Validate that the appropriate bounds are respected when a Cartesian product uses the same index in two streams
    def test03_cartesian_product_reused_index(self):
        redis_graph.query("CREATE INDEX FOR ()-[f:friend]-() ON (f.updated_at)")
        query = "MATCH ()-[a:friend]->(), ()-[b:friend]->() WHERE a.created_at >= 80 AND b.updated_at >= 120 RETURN a.created_at, b.updated_at"
        plan = redis_graph.execution_plan(query)
        # The two streams should both use index scans
        self.env.assertEquals(plan.count('Edge By Index Scan'), 2)
        self.env.assertNotIn('Conditional Traverse', plan)


        expected_result = [[81, 120], [83, 120], [81, 123], [83, 123]]
        result = redis_graph.query(query)

        self.env.assertEquals(result.result_set, expected_result)

    # Validate index utilization when filtering on a numeric field with the `IN` keyword.
    def test04_test_in_operator_numerics(self):
        # Validate the transformation of IN to multiple OR expressions.
        query = "MATCH ()-[f:friend]-() WHERE f.created_at IN [1,2,3] RETURN f"
        plan = redis_graph.execution_plan(query)
        self.env.assertIn('Edge By Index Scan', plan)

        # Validate that nested arrays are not scanned in index.
        query = "MATCH ()-[f:friend]-() WHERE f.created_at IN [[1,2],3] RETURN f"
        plan = redis_graph.execution_plan(query)
        self.env.assertNotIn('Edge By Index Scan', plan)
        self.env.assertIn('Conditional Traverse', plan)

        # Validate the transformation of IN to multiple OR, over a range.
        query = "MATCH (n)-[f:friend]->() WHERE f.created_at IN range(0,30) RETURN DISTINCT n.name ORDER BY n.name"
        plan = redis_graph.execution_plan(query)
        self.env.assertIn('Edge By Index Scan', plan)

        expected_result = [['Ailon'], ['Alon'], ['Roi']]
        result = redis_graph.query(query)
        self.env.assertEquals(result.result_set, expected_result)

         # Validate the transformation of IN to empty index iterator.
        query = "MATCH ()-[f:friend]-() WHERE f.created_at IN [] RETURN f.name"
        plan = redis_graph.execution_plan(query)
        self.env.assertIn('Edge By Index Scan', plan)

        expected_result = []
        result = redis_graph.query(query)
        self.env.assertEquals(result.result_set, expected_result)

        # Validate the transformation of IN OR IN to empty index iterators.
        query = "MATCH ()-[f:friend]->() WHERE f.created_at IN [] OR f.created_at IN [] RETURN f.name"
        plan = redis_graph.execution_plan(query)
        self.env.assertIn('Edge By Index Scan', plan)

        expected_result = []
        result = redis_graph.query(query)
        self.env.assertEquals(result.result_set, expected_result)

        # Validate the transformation of multiple IN filters.
        query = "MATCH (n)-[f:friend]->() WHERE f.created_at IN [0, 1, 2] OR f.created_at IN [14, 15, 16] RETURN n.name ORDER BY n.name"
        plan = redis_graph.execution_plan(query)
        self.env.assertIn('Edge By Index Scan', plan)

        expected_result = [['Alon'], ['Roi']]
        result = redis_graph.query(query)
        self.env.assertEquals(result.result_set, expected_result)

        # Validate the transformation of multiple IN filters.
        query = "MATCH (n)-[f:friend]->() WHERE f.created_at IN [0, 1, 2] OR f.created_at IN [14, 15, 16] OR f.created_at IN [] RETURN n.name ORDER BY n.name"
        plan = redis_graph.execution_plan(query)
        self.env.assertIn('Edge By Index Scan', plan)

        result = redis_graph.query(query)
        self.env.assertEquals(result.result_set, expected_result)

    # # Validate index utilization when filtering on string fields with the `IN` keyword.
    # def test05_test_in_operator_string_props(self):
    #     # Build an index on the name property.
    #     redis_graph.query("CREATE INDEX ON :person(name)")
    #     # Validate the transformation of IN to multiple OR expressions over string properties.
    #     query = "MATCH (p:person) WHERE p.name IN ['Gal Derriere', 'Lucy Yanfital'] RETURN p.name ORDER BY p.name"
    #     plan = redis_graph.execution_plan(query)
    #     self.env.assertIn('Index Scan', plan)
    #     self.env.assertNotIn('Label Scan', plan)

    #     expected_result = [['Gal Derriere'], ['Lucy Yanfital']]
    #     result = redis_graph.query(query)
    #     self.env.assertEquals(result.result_set, expected_result)

    #     # Combine numeric and string filters specified by IN.
    #     query = "MATCH (p:person) WHERE p.name IN ['Gal Derriere', 'Lucy Yanfital'] AND p.age in [30] RETURN p.name ORDER BY p.name"
    #     plan = redis_graph.execution_plan(query)
    #     self.env.assertIn('Index Scan', plan)
    #     self.env.assertNotIn('Label Scan', plan)

    #     expected_result = [['Lucy Yanfital']]
    #     result = redis_graph.query(query)
    #     self.env.assertEquals(result.result_set, expected_result)

    #      # Validate an empty index on IN with multiple indexes
    #     query = "MATCH (p:person) WHERE p.name IN [] OR p.age IN [] RETURN p.name"
    #     plan = redis_graph.execution_plan(query)
    #     self.env.assertIn('Index Scan', plan)

    #     expected_result = []
    #     result = redis_graph.query(query)
    #     self.env.assertEquals(result.result_set, expected_result)

    #     # Combine IN filters with other relational filters.
    #     query = "MATCH (p:person) WHERE p.name IN ['Gal Derriere', 'Lucy Yanfital'] AND p.name < 'H' RETURN p.name ORDER BY p.name"
    #     plan = redis_graph.execution_plan(query)
    #     self.env.assertIn('Index Scan', plan)
    #     self.env.assertNotIn('Label Scan', plan)

    #     expected_result = [['Gal Derriere']]
    #     result = redis_graph.query(query)
    #     self.env.assertEquals(result.result_set, expected_result)

    #     query = "MATCH (p:person) WHERE p.name IN ['Gal Derriere', 'Lucy Yanfital'] OR p.age = 33 RETURN p.name ORDER BY p.name"
    #     plan = redis_graph.execution_plan(query)
    #     self.env.assertIn('Index Scan', plan)
    #     self.env.assertNotIn('Label Scan', plan)

    #     expected_result = [['Gal Derriere'], ['Lucy Yanfital'], ['Omri Traub']]
    #     result = redis_graph.query(query)
    #     result = redis_graph.query(query)
    #     self.env.assertEquals(result.result_set, expected_result)

    # # ',' is the default separator for tag indices
    # # we've updated our separator to '\0' this test verifies issue 696:
    # # https://github.com/RedisGraph/RedisGraph/issues/696
    # def test06_tag_separator(self):
    #     redis_con = self.env.getConnection()
    #     redis_graph = Graph("G", redis_con)

    #     # Create a single node with a long string property, introduce a comma as part of the string.
    #     query = """CREATE (:Node{value:"A ValuePartition is a pattern that describes a restricted set of classes from which a property can be associated. The parent class is used in restrictions, and the covering axiom means that only members of the subclasses may be used as values."})"""
    #     redis_graph.query(query)

    #     # Index property.
    #     query = """CREATE INDEX ON :Node(value)"""
    #     redis_graph.query(query)

    #     # Make sure node is returned by index scan.
    #     query = """MATCH (a:Node{value:"A ValuePartition is a pattern that describes a restricted set of classes from which a property can be associated. The parent class is used in restrictions, and the covering axiom means that only members of the subclasses may be used as values."}) RETURN a"""
    #     plan = redis_graph.execution_plan(query)
    #     result_set = redis_graph.query(query).result_set
    #     self.env.assertIn('Index Scan', plan)
    #     self.env.assertEqual(len(result_set), 1)

    def test07_index_scan_and_id(self):
        query = """MATCH (n)-[f:friend]->() WHERE id(f)>=10 AND f.created_at<15 RETURN n.name ORDER BY n.name"""
        plan = redis_graph.execution_plan(query)
        query_result = redis_graph.query(query)
        self.env.assertIn('Edge By Index Scan', plan)
        self.env.assertIn('Filter', plan)
        query_result = redis_graph.query(query)

        self.env.assertEqual(2, len(query_result.result_set))
        expected_result = [['Alon'], ['Roi']]
        self.env.assertEquals(expected_result, query_result.result_set)

    # Validate placement of index scans and filter ops when not all filters can be replaced.
    def test08_index_scan_multiple_filters(self):
        query = "MATCH (n)-[f:friend]->() WHERE f.created_at = 31 AND NOT EXISTS(f.fakeprop) RETURN n.name"
        plan = redis_graph.execution_plan(query)
        self.env.assertIn('Edge By Index Scan', plan)
        self.env.assertNotIn('Conditional Traverse', plan)
        self.env.assertIn('Filter', plan)

        query_result = redis_graph.query(query)
        expected_result = ["Ailon"]
        self.env.assertEquals(query_result.result_set[0], expected_result)

    def test09_index_scan_with_params(self):
        query = "MATCH (n)-[f:friend]->() WHERE f.created_at = $time RETURN n.name"
        params = {'time': 31}
        plan = redis_graph.execution_plan(query, params=params)
        self.env.assertIn('Edge By Index Scan', plan)
        query_result = redis_graph.query(query, params=params)
        expected_result = ["Ailon"]
        self.env.assertEquals(query_result.result_set[0], expected_result)

    def test10_index_scan_with_param_array(self):
        query = "MATCH (n)-[f:friend]->() WHERE f.created_at in $times RETURN n.name"
        params = {'times': [31]}
        plan = redis_graph.execution_plan(query, params=params)
        self.env.assertIn('Edge By Index Scan', plan)
        query_result = redis_graph.query(query, params=params)
        expected_result = ["Ailon"]
        self.env.assertEquals(query_result.result_set[0], expected_result)

    def test11_single_index_multiple_scans(self):
        query = "MATCH (p1:person {name: 'Roi'}), (p2:person {name: 'Alon'}) MERGE (p1)-[:friend {created_at: 100}]->(p2) MERGE (p1)-[:friend {created_at: 101}]->(p2)"
        plan = redis_graph.execution_plan(query)
        # Two index scans should be performed.
        self.env.assertEqual(plan.count("Edge By Index Scan"), 2)

        query_result = redis_graph.query(query)
        # Two new nodes should be created.
        self.env.assertEquals(query_result.relationships_created, 2)

    # def test12_remove_scans_before_index(self):
    #     query = "MATCH (a:person {age: 32})-[]->(b) WHERE (b:person)-[]->(a) RETURN a"
    #     plan = redis_graph.execution_plan(query)
    #     # One index scan should be performed.
    #     self.env.assertEqual(plan.count("Index Scan"), 1)

    # def test13_point_index_scan(self):
    #     # create index
    #     q = "CREATE INDEX ON :restaurant(location)"
    #     redis_graph.query(q)

    #     # create restaurant
    #     q = "CREATE (:restaurant {location: point({latitude:30.27822306, longitude:-97.75134723})})"
    #     redis_graph.query(q)

    #     # locate other restaurants within a 1000m radius
    #     q = """MATCH (r:restaurant)
    #     WHERE distance(r.location, point({latitude:30.27822306, longitude:-97.75134723})) < 1000
    #     RETURN r"""

    #     # make sure index is used
    #     plan = redis_graph.execution_plan(q)
    #     self.env.assertIn("Index Scan", plan)

    #     # refine query from '<' to '<='
    #     q = """MATCH (r:restaurant)
    #     WHERE distance(r.location, point({latitude:30.27822306, longitude:-97.75134723})) <= 1000
    #     RETURN r"""

    #     # make sure index is used
    #     plan = redis_graph.execution_plan(q)
    #     self.env.assertIn("Index Scan", plan)

    #     # index should NOT be used when searching for points outside of a circle
    #     # testing operand: '>', '>=' and '='
    #     q = """MATCH (r:restaurant)
    #     WHERE distance(r.location, point({latitude:30.27822306, longitude:-97.75134723})) > 1000
    #     RETURN r"""

    #     # make sure index is NOT used
    #     plan = redis_graph.execution_plan(q)
    #     self.env.assertNotIn("Index Scan", plan)

    #     q = """MATCH (r:restaurant)
    #     WHERE distance(r.location, point({latitude:30.27822306, longitude:-97.75134723})) >= 1000
    #     RETURN r"""

    #     # make sure index is NOT used
    #     plan = redis_graph.execution_plan(q)
    #     self.env.assertNotIn("Index Scan", plan)

    #     q = """MATCH (r:restaurant)
    #     WHERE distance(r.location, point({latitude:30.27822306, longitude:-97.75134723})) = 1000
    #     RETURN r"""

    #     # make sure index is NOT used
    #     plan = redis_graph.execution_plan(q)
    #     self.env.assertNotIn("Index Scan", plan)

    # def test14_index_scan_utilize_array(self):
    #     # Querying indexed properties using IN a constant array should utilize indexes.
    #     query = "MATCH (a:person) WHERE a.age IN [34, 33] RETURN a.name ORDER BY a.name"
    #     plan = redis_graph.execution_plan(query)
    #     # One index scan should be performed.
    #     self.env.assertEqual(plan.count("Index Scan"), 1)
    #     query_result = redis_graph.query(query)
    #     expected_result = [["Noam Nativ"],
    #                        ["Omri Traub"]]
    #     self.env.assertEquals(query_result.result_set, expected_result)

    #     # Querying indexed properties using IN a generated array should utilize indexes.
    #     query = "MATCH (a:person) WHERE a.age IN range(33, 34) RETURN a.name ORDER BY a.name"
    #     plan = redis_graph.execution_plan(query)
    #     # One index scan should be performed.
    #     self.env.assertEqual(plan.count("Index Scan"), 1)
    #     query_result = redis_graph.query(query)
    #     expected_result = [["Noam Nativ"],
    #                        ["Omri Traub"]]
    #     self.env.assertEquals(query_result.result_set, expected_result)

    #     # Querying indexed properties using IN a non-constant array should not utilize indexes.
    #     query = "MATCH (a:person)-[]->(b) WHERE a.age IN b.arr RETURN a"
    #     plan = redis_graph.execution_plan(query)
    #     # No index scans should be performed.
    #     self.env.assertEqual(plan.count("Label Scan"), 1)
    #     self.env.assertEqual(plan.count("Index Scan"), 0)

    def test16_runtime_index_utilization(self):
        # find all person nodes with age in the range 33-37
        # current age (x) should be resolved at runtime
        # index query should be constructed for each age value
        q = """UNWIND range(33, 37) AS x
        MATCH (n)-[f:friend {created_at: x}]->()
        RETURN n.name
        ORDER BY n.name"""
        plan = redis_graph.execution_plan(q)
        self.env.assertIn('Edge By Index Scan', plan)
        query_result = redis_graph.query(q)
        expected_result = [['Ailon'], ['Ailon'], ['Boaz']]
        self.env.assertEquals(query_result.result_set, expected_result)

        # similar to the query above, only this time the filter is specified
        # by an OR condition
        q = """WITH 33 AS min, 37 AS max 
        MATCH (n)-[f:friend]->()
        WHERE f.created_at = min OR f.created_at = max
        RETURN n.name
        ORDER BY n.name"""
        plan = redis_graph.execution_plan(q)
        self.env.assertIn('Edge By Index Scan', plan)
        query_result = redis_graph.query(q)
        expected_result = [['Ailon'], ['Boaz']]
        self.env.assertEquals(query_result.result_set, expected_result)

        # find all person nodes with age equals 33 'x'
        # 'x' value is known only at runtime
        q = """WITH 33 AS x
        MATCH (n)-[f:friend {created_at: x}]->()
        RETURN n.name
        ORDER BY n.name"""
        plan = redis_graph.execution_plan(q)
        self.env.assertIn('Edge By Index Scan', plan)
        query_result = redis_graph.query(q)
        expected_result = [["Ailon"]]
        self.env.assertEquals(query_result.result_set, expected_result)

        # find all person nodes with age equals x + 1
        # the expression x+1 is evaluated to the constant 33 only at runtime
        # expecting index query to be constructed at runtime
        q = """WITH 32 AS x
        MATCH (n)-[f:friend]->()
        WHERE f.created_at = (x + 1)
        RETURN n.name
        ORDER BY n.name"""
        plan = redis_graph.execution_plan(q)
        self.env.assertIn('Edge By Index Scan', plan)
        query_result = redis_graph.query(q)
        expected_result = [["Ailon"]]
        self.env.assertEquals(query_result.result_set, expected_result)

        # same idea as previous query only we've switched the position of the
        # operands, queried entity (p.age) is now on the right hand side of the
        # filter, expecting the same behavior
        q = """WITH 32 AS x
        MATCH (n)-[f:friend]->()
        WHERE (x + 1) = f.created_at
        RETURN n.name
        ORDER BY n.name"""
        plan = redis_graph.execution_plan(q)
        self.env.assertIn('Edge By Index Scan', plan)
        query_result = redis_graph.query(q)
        expected_result = [["Ailon"]]
        self.env.assertEquals(query_result.result_set, expected_result)

        # # find all person nodes 'b' with age greater than node 'a'
        # # a's age value is determined only at runtime
        # # expecting index to be used to resolve 'b' nodes, index query should be
        # # constructed at runtime
        # q = """MATCH (a:person {name:'Omri Traub'})
        # WITH a AS a
        # MATCH (b:person)
        # WHERE b.age > a.age
        # RETURN b.name
        # ORDER BY b.name"""
        # plan = redis_graph.execution_plan(q)
        # self.env.assertIn('Index Scan', plan)
        # query_result = redis_graph.query(q)
        # expected_result = [["Noam Nativ"]]
        # self.env.assertEquals(query_result.result_set, expected_result)

        # # same idea as previous query, only this time we've switched filter
        # # operands position, queries entity is on the right hand side
        # q = """MATCH (a:person {name: 'Omri Traub'})
        # WITH a AS a
        # MATCH (b:person)
        # WHERE a.age < b.age
        # RETURN b.name
        # ORDER BY b.name"""
        # plan = redis_graph.execution_plan(q)
        # self.env.assertIn('Index Scan', plan)
        # query_result = redis_graph.query(q)
        # expected_result = [["Noam Nativ"]]
        # self.env.assertEquals(query_result.result_set, expected_result)

        # # TODO: The following query uses the "Value Hash Join" where it would be
        # # better to use "Index Scan"
        # q = """UNWIND range(33, 37) AS x MATCH (a:person {age:x}), (b:person {age:x}) RETURN a.name, b.name ORDER BY a.name, b.name"""

    # def test17_runtime_index_utilization_array_values(self):
    #     # when constructing an index query at runtime it is possible to encounter
    #     # none indexable values e.g. Array, in which case the index will still be
    #     # utilize, producing every entity which was indexed with a none indexable value
    #     # to which the index scan operation will have to apply the original filter

    #     # create person nodes with array value for their 'age' attribute
    #     q = """CREATE (:person {age:[36], name:'leonard'}), (:person {age:[34], name:['maynard']})"""
    #     redis_graph.query(q)

    #     # find all person nodes with age value of [36]
    #     q = """WITH [36] AS age MATCH (a:person {age:age}) RETURN a.name"""
    #     plan = redis_graph.execution_plan(q)
    #     self.env.assertIn('Index Scan', plan)
    #     query_result = redis_graph.query(q)
    #     expected_result = [["leonard"]]
    #     self.env.assertEquals(query_result.result_set, expected_result)

    #     # find all person nodes with age > [33]
    #     q = """WITH [33] AS age MATCH (a:person) WHERE a.age > age RETURN a.name"""
    #     plan = redis_graph.execution_plan(q)
    #     self.env.assertIn('Index Scan', plan)
    #     query_result = redis_graph.query(q)
    #     expected_result = [["leonard"], [["maynard"]]]
    #     self.env.assertEquals(query_result.result_set, expected_result)

    #     # combine indexable value with none-indexable value index query
    #     q = """WITH [33] AS age, 'leonard' AS name MATCH (a:person) WHERE a.age >= age AND a.name = name RETURN a.name"""
    #     plan = redis_graph.execution_plan(q)
    #     self.env.assertIn('Index Scan', plan)
    #     query_result = redis_graph.query(q)
    #     expected_result = [["leonard"]]
    #     self.env.assertEquals(query_result.result_set, expected_result)

    def test_18_index_scan_and_label_filter(self):
        query = "MATCH (n)-[f:friend]->(m) WHERE f.created_at = 1 RETURN n.name"
        plan = redis_graph.execution_plan(query)
        self.env.assertIn('Edge By Index Scan', plan)
        self.env.assertNotIn('All Node Scan', plan)
        query_result = redis_graph.query(query)
        expected_result = ["Roi"]
        self.env.assertEquals(query_result.result_set[0], expected_result)

        query = "MATCH (n:person)-[f:friend]->(m) WHERE f.created_at = 1 RETURN n.name"
        plan = redis_graph.execution_plan(query)
        self.env.assertIn('Edge By Index Scan', plan)
        self.env.assertIn('Node By Label Scan', plan)
        query_result = redis_graph.query(query)
        expected_result = ["Roi"]
        self.env.assertEquals(query_result.result_set[0], expected_result)

        query = "MATCH (n:person)-[f:friend]->(m:person) WHERE f.created_at = 1 RETURN n.name"
        plan = redis_graph.execution_plan(query)
        self.env.assertIn('Edge By Index Scan', plan)
        self.env.assertIn('Node By Label Scan', plan)
        self.env.assertIn('Filter', plan)
        query_result = redis_graph.query(query)
        expected_result = ["Roi"]
        self.env.assertEquals(query_result.result_set[0], expected_result)

        query = "MATCH (n:person {name: 'Roi'})-[f:friend]->(m:person) WHERE f.created_at = 1 RETURN n.name"
        plan = redis_graph.execution_plan(query)
        self.env.assertIn('Edge By Index Scan', plan)
        self.env.assertIn('Node By Label Scan', plan)
        self.env.assertIn('Filter', plan)
        query_result = redis_graph.query(query)
        expected_result = ["Roi"]
        self.env.assertEquals(query_result.result_set[0], expected_result)

        query = "MATCH (n:person {name: 'Alon'})-[f:friend]->(m:person) WHERE f.created_at = 1 RETURN n.name"
        plan = redis_graph.execution_plan(query)
        self.env.assertIn('Edge By Index Scan', plan)
        self.env.assertIn('Node By Label Scan', plan)
        self.env.assertIn('Filter', plan)
        query_result = redis_graph.query(query)
        self.env.assertEquals(query_result.result_set, [])

        query = "MATCH (n)<-[f:friend]-(m) WHERE f.created_at = 1 RETURN n.name"
        plan = redis_graph.execution_plan(query)
        self.env.assertIn('Edge By Index Scan', plan)
        self.env.assertNotIn('All Node Scan', plan)
        query_result = redis_graph.query(query)
        expected_result = ["Alon"]
        self.env.assertEquals(query_result.result_set[0], expected_result)

        query = "MATCH (n:person)<-[f:friend]-(m) WHERE f.created_at = 1 RETURN n.name"
        plan = redis_graph.execution_plan(query)
        self.env.assertIn('Edge By Index Scan', plan)
        self.env.assertIn('Node By Label Scan', plan)
        query_result = redis_graph.query(query)
        expected_result = ["Alon"]
        self.env.assertEquals(query_result.result_set[0], expected_result)

        query = "MATCH (n:person)<-[f:friend]-(m:person) WHERE f.created_at = 1 RETURN n.name"
        plan = redis_graph.execution_plan(query)
        self.env.assertIn('Edge By Index Scan', plan)
        self.env.assertIn('Node By Label Scan', plan)
        self.env.assertIn('Filter', plan)
        query_result = redis_graph.query(query)
        expected_result = ["Alon"]
        self.env.assertEquals(query_result.result_set[0], expected_result)

        query = "MATCH (n:person {name: 'Roi'})<-[f:friend]-(m:person) WHERE f.created_at = 1 RETURN n.name"
        plan = redis_graph.execution_plan(query)
        self.env.assertIn('Edge By Index Scan', plan)
        self.env.assertIn('Node By Label Scan', plan)
        self.env.assertIn('Filter', plan)
        query_result = redis_graph.query(query)
        self.env.assertEquals(query_result.result_set, [])

        query = "MATCH (n:person {name: 'Alon'})<-[f:friend]-(m:person) WHERE f.created_at = 1 RETURN n.name"
        plan = redis_graph.execution_plan(query)
        self.env.assertIn('Edge By Index Scan', plan)
        self.env.assertIn('Node By Label Scan', plan)
        self.env.assertIn('Filter', plan)
        query_result = redis_graph.query(query)
        expected_result = ["Alon"]
        self.env.assertEquals(query_result.result_set[0], expected_result)
