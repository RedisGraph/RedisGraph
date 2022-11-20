from common import *

graph = None
redis_con = None
people = ["Roi", "Alon", "Ailon", "Boaz"]


class testOptimizationsPlan(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global graph
        global redis_con
        redis_con = self.env.getConnection()
        graph = Graph(redis_con, "g")
        self.populate_graph()

    def populate_graph(self):
        global graph
        nodes = {}
        # Create entities
        for idx, p in enumerate(people):
            node = Node(label="person", properties={"name": p, "val": idx})
            graph.add_node(node)
            nodes[p] = node

        # Fully connected graph
        for src in nodes:
            for dest in nodes:
                if src != dest:
                    edge = Edge(nodes[src], "know", nodes[dest])
                    graph.add_edge(edge)

        for src in nodes:
            for dest in nodes:
                if src != dest:
                    edge = Edge(nodes[src], "works_with", nodes[dest])
                    graph.add_edge(edge)

        graph.commit()
        query = """MATCH (a)-[:know]->(b) CREATE (a)-[:know]->(b)"""
        graph.query(query)

    def test01_typeless_edge_count(self):
        query = """MATCH ()-[r]->() RETURN COUNT(r)"""
        resultset = graph.query(query).result_set
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Project", executionPlan)
        self.env.assertIn("Results", executionPlan)
        self.env.assertNotIn("All Node Scan", executionPlan)
        self.env.assertNotIn("Conditional Traverse", executionPlan)
        self.env.assertNotIn("Aggregate", executionPlan)
        expected = [[36]]
        self.env.assertEqual(resultset, expected)

    def test02_typed_edge_count(self):
        query = """MATCH ()-[r:know]->() RETURN COUNT(r)"""
        resultset = graph.query(query).result_set
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Project", executionPlan)
        self.env.assertIn("Results", executionPlan)
        self.env.assertNotIn("All Node Scan", executionPlan)
        self.env.assertNotIn("Conditional Traverse", executionPlan)
        self.env.assertNotIn("Aggregate", executionPlan)
        expected = [[24]]
        self.env.assertEqual(resultset, expected)

    def test03_unknown_typed_edge_count(self):
        query = """MATCH ()-[r:unknown]->() RETURN COUNT(r)"""
        resultset = graph.query(query).result_set
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Project", executionPlan)
        self.env.assertIn("Results", executionPlan)
        self.env.assertNotIn("All Node Scan", executionPlan)
        self.env.assertNotIn("Conditional Traverse", executionPlan)
        self.env.assertNotIn("Aggregate", executionPlan)
        expected = [[0]]
        self.env.assertEqual(resultset, expected)

    def test04_typeless_edge_count_with_alias(self):
        query = """MATCH ()-[r]->() RETURN COUNT(r) as c"""
        resultset = graph.query(query).result_set
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Project", executionPlan)
        self.env.assertIn("Results", executionPlan)
        self.env.assertNotIn("All Node Scan", executionPlan)
        self.env.assertNotIn("Conditional Traverse", executionPlan)
        self.env.assertNotIn("Aggregate", executionPlan)
        expected = [[36]]
        self.env.assertEqual(resultset, expected)

    def test05_typed_edge_count_with_alias(self):
        query = """MATCH ()-[r:know]->() RETURN COUNT(r) as c"""
        resultset = graph.query(query).result_set
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Project", executionPlan)
        self.env.assertIn("Results", executionPlan)
        self.env.assertNotIn("All Node Scan", executionPlan)
        self.env.assertNotIn("Conditional Traverse", executionPlan)
        self.env.assertNotIn("Aggregate", executionPlan)
        expected = [[24]]
        self.env.assertEqual(resultset, expected)

    def test06_multiple_typed_edge_count_with_alias(self):
        query = """MATCH ()-[r:know | :works_with]->() RETURN COUNT(r) as c"""
        resultset = graph.query(query).result_set
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Project", executionPlan)
        self.env.assertIn("Results", executionPlan)
        self.env.assertNotIn("All Node Scan", executionPlan)
        self.env.assertNotIn("Conditional Traverse", executionPlan)
        self.env.assertNotIn("Aggregate", executionPlan)
        expected = [[36]]
        self.env.assertEqual(resultset, expected)

    def test07_count_unreferenced_edge(self):
        query = """MATCH ()-[:know]->(b) RETURN COUNT(b)"""
        # This count in this query cannot be reduced, as the traversal op doesn't store
        # data about non-referenced edges.
        resultset = graph.query(query).result_set
        executionPlan = graph.execution_plan(query)
        # Verify that the optimization was not applied.
        self.env.assertNotIn("Project", executionPlan)
        self.env.assertIn("Aggregate", executionPlan)
        self.env.assertIn("All Node Scan", executionPlan)
        self.env.assertIn("Conditional Traverse", executionPlan)
        expected = [[12]]
        self.env.assertEqual(resultset, expected)

    def test08_non_labeled_node_count(self):
        query = """MATCH (n) RETURN COUNT(n)"""
        resultset = graph.query(query).result_set
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Project", executionPlan)
        self.env.assertIn("Results", executionPlan)
        self.env.assertNotIn("All Node Scan", executionPlan)
        self.env.assertNotIn("Node By Label Scan", executionPlan)
        self.env.assertNotIn("Aggregate", executionPlan)
        expected = [[4]]
        self.env.assertEqual(resultset, expected)

    def test09_non_labeled_node_count_with_alias(self):
        query = """MATCH (n) RETURN COUNT(n) as c"""
        resultset = graph.query(query).result_set
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Project", executionPlan)
        self.env.assertIn("Results", executionPlan)
        self.env.assertNotIn("All Node Scan", executionPlan)
        self.env.assertNotIn("Node By Label Scan", executionPlan)
        self.env.assertNotIn("Aggregate", executionPlan)
        expected = [[4]]
        self.env.assertEqual(resultset, expected)

    def test10_labled_node_count(self):
        query = """MATCH (n:person) RETURN COUNT(n)"""
        resultset = graph.query(query).result_set
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Project", executionPlan)
        self.env.assertIn("Results", executionPlan)
        self.env.assertNotIn("All Node Scan", executionPlan)
        self.env.assertNotIn("Node By Label Scan", executionPlan)
        self.env.assertNotIn("Aggregate", executionPlan)
        expected = [[4]]
        self.env.assertEqual(resultset, expected)

    def test11_value_hash_join(self):
        # Issue a query that joins two streams on a node property.
        query = """MATCH (p1:person)-[:know]->({name: 'Roi'}), (p2)-[]->(:person {name: 'Alon'}) WHERE p1.name = p2.name RETURN p2.name ORDER BY p2.name"""
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Value Hash Join", executionPlan)
        self.env.assertNotIn("Cartesian Product", executionPlan)

        resultset = graph.query(query).result_set
        expected = [['Ailon'], ['Boaz']]
        self.env.assertEqual(resultset, expected)

        # Issue a query that joins two streams on a function call.
        query = """MATCH (p1:person)-[:know]->({name: 'Roi'}) MATCH (p2)-[]->(:person {name: 'Alon'}) WHERE ID(p1) = ID(p2) RETURN p2.name ORDER BY p2.name"""
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Value Hash Join", executionPlan)
        self.env.assertNotIn("Cartesian Product", executionPlan)

        resultset = graph.query(query).result_set
        self.env.assertEqual(resultset, expected) # same results expected

        query = """MATCH (p1:person)-[:know]->({name: 'Roi'}) MATCH (p2)-[]->(:person {name: 'Alon'}) WHERE p1 = p2 RETURN p2.name ORDER BY p2.name"""
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Value Hash Join", executionPlan)
        self.env.assertNotIn("Cartesian Product", executionPlan)

        resultset = graph.query(query).result_set
        self.env.assertEqual(resultset, expected) # same results expected

    def test12_multiple_stream_value_hash_join(self):
        # Issue a query that joins three streams.
        query = """MATCH (p1:person)-[:know]->({name: 'Roi'}), (p2)-[]->(:person {name: 'Alon'}), (p3) WHERE p1.name = p2.name AND ID(p2) = ID(p3) RETURN p2.name ORDER BY p2.name"""
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Value Hash Join", executionPlan)
        self.env.assertNotIn("Cartesian Product", executionPlan)

        resultset = graph.query(query).result_set
        expected = [['Ailon'], ['Boaz']]
        self.env.assertEqual(resultset, expected)

        # Issue a query that joins four streams that all resolve the same entity.
        query = """MATCH (p1 {name: 'Ailon'}), (p2), (p3), (p4) WHERE ID(p1) = ID(p2) AND ID(p2) = ID(p3) AND p3.name = p4.name RETURN p4.name"""
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Value Hash Join", executionPlan)
        self.env.assertNotIn("Cartesian Product", executionPlan)

        expected = [['Ailon']]
        resultset = graph.query(query).result_set
        self.env.assertEqual(resultset, expected)

        # Issue a query that joins four streams that all resolve the same entity, with multiple reapeating filter (issue #869).
        query = """MATCH (p1 {name: 'Ailon'}), (p2), (p3), (p4) WHERE ID(p1) = ID(p2) AND ID(p2) = ID(p3) AND ID(p3)=ID(p2) AND ID(p2)= ID(p1) AND p3.name = p4.name AND p4.name = p3.name RETURN p4.name"""
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Value Hash Join", executionPlan)
        self.env.assertNotIn("Cartesian Product", executionPlan)

        expected = [['Ailon']]
        resultset = graph.query(query).result_set
        self.env.assertEqual(resultset, expected)

    def test13_duplicate_filter_placement(self):
        # Issue a query that joins three streams and contains a redundant filter.
        query = """MATCH (p0), (p1), (p2) where id(p2) = id(p0) AND id(p1) = id(p2) AND id(p1) = id(p2) return p2.name ORDER BY p2.name"""
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Value Hash Join", executionPlan)
        self.env.assertNotIn("Cartesian Product", executionPlan)

        resultset = graph.query(query).result_set
        expected = [['Ailon'], ['Alon'], ['Boaz'], ['Roi']]
        self.env.assertEqual(resultset, expected)

    def test14_distinct_aggregations(self):
        # Verify that the Distinct operation is removed from the aggregating query.
        query = """MATCH (src:person)-[:know]->(dest) RETURN DISTINCT src.name, COUNT(dest) ORDER BY src.name"""
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Aggregate", executionPlan)
        self.env.assertNotIn("Distinct", executionPlan)

        resultset = graph.query(query).result_set
        expected = [['Ailon', 3],
                    ['Alon', 3],
                    ['Boaz', 3],
                    ['Roi', 3]]
        self.env.assertEqual(resultset, expected)


        # Verify that the Distinct operation is not removed from a valid projection.
        query = """MATCH (src:person) WITH DISTINCT src MATCH (src)-[:know]->(dest) RETURN src.name, COUNT(dest) ORDER BY src.name"""
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Aggregate", executionPlan)
        self.env.assertIn("Distinct", executionPlan)

        resultset = graph.query(query).result_set
        # This query should emit the same result.
        self.env.assertEqual(resultset, expected)

    def test15_test_splitting_cartesian_product(self):
        query = """MATCH (p1), (p2), (p3) WHERE p1.name <> p2.name AND p2.name <> p3.name RETURN DISTINCT p2.name ORDER BY p2.name"""
        executionPlan = graph.execution_plan(query)
        self.env.assertEqual(2, executionPlan.count("Cartesian Product"))
        expected = [['Ailon'],
                    ['Alon'],
                    ['Boaz'],
                    ['Roi']]
        resultset = graph.query(query).result_set
        self.env.assertEqual(resultset, expected)
    
    def test16_test_splitting_cartesian_product_with_multiple_filters(self):
        query = """MATCH (p1), (p2), (p3) WHERE p1.name <> p2.name AND ID(p1) <> ID(p2) RETURN DISTINCT p2.name ORDER BY p2.name"""
        executionPlan = graph.execution_plan(query)
        self.env.assertEqual(2, executionPlan.count("Cartesian Product"))
        expected = [['Ailon'],
                    ['Alon'],
                    ['Boaz'],
                    ['Roi']]
        resultset = graph.query(query).result_set
        self.env.assertEqual(resultset, expected)

    def test17_test_multiple_branch_filter_cp_optimization(self):
        query = """MATCH (p1), (p2), (p3), (p4) WHERE p1.val + p2.val = p3.val AND p3.val > 0 RETURN DISTINCT p3.name ORDER BY p3.name"""
        executionPlan = graph.execution_plan(query)
        self.env.assertEqual(2, executionPlan.count("Cartesian Product"))
        expected = [['Ailon'],
                    ['Alon'],
                    ['Boaz']]
        resultset = graph.query(query).result_set
        self.env.assertEqual(resultset, expected)

    def test18_test_semi_apply_and_cp_optimize(self):
        graph.query ("CREATE ({val:0}), ({val:1})-[:R]->({val:2})-[:R]->({val:3})")
        # The next query generates the execution plan:
        # 1) "Results"
        # 2) "    Sort"
        # 3) "        Distinct"
        # 4) "            Project"
        # 5) "                Semi Apply"
        # 6) "                    Cartesian Product"
        # 7) "                        All Node Scan | (n4)"
        # 8) "                        Filter"
        # 9) "                            Cartesian Product"
        # 10) "                                All Node Scan | (n1)"
        # 11) "                                Filter"
        # 12) "                                    All Node Scan | (n3)"
        # 13) "                                All Node Scan | (n2)"
        # 14) "                    Expand Into | (n3)->(n4)"
        # 15) "                        Filter"
        # 16) "                            Argument"
        # We want to make sure the optimization is not misplacing the semi apply bounded branch.
        resultset = graph.query("MATCH (n1), (n2), (n3), (n4) WHERE (n3)-[:R]->(n4 {val:n3.val+1}) AND n1.val + n2.val = n3.val AND n3.val > 1  RETURN DISTINCT n3.val ORDER BY n3.val").result_set
        expected = [[2]]
        self.env.assertEqual(resultset, expected)
    
    def test19_test_filter_compaction_remove_true_filter(self):
        query = "MATCH (n) WHERE 1 = 1 RETURN n"
        executionPlan = graph.execution_plan(query)
        self.env.assertNotIn("Filter", executionPlan)

    def test20_test_filter_compaction_not_removing_false_filter(self):
        query = "MATCH (n) WHERE 1 > 1 RETURN n"
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Filter", executionPlan)
        resultset = graph.query(query).result_set
        expected = []
        self.env.assertEqual(resultset, expected)

    # ExpandInto should be applied where possible on projected graph entities.
    def test21_expand_into_projected_endpoints(self):
        query = """MATCH (a)-[]->(b) WITH a, b MATCH (a)-[e]->(b) RETURN a.val, b.val ORDER BY a.val, b.val LIMIT 3"""
        executionPlan = graph.execution_plan(query)
        self.env.assertIn("Expand Into", executionPlan)
        resultset = graph.query(query).result_set
        expected = [[0, 1],
                    [0, 2],
                    [0, 3]]
        self.env.assertEqual(resultset, expected)

    # Variables bound in one scope should not be used to introduce ExpandInto ops in later scopes.
    def test22_no_expand_into_across_scopes(self):
        query = """MATCH (reused_1)-[]->(reused_2) WITH COUNT(reused_2) as edge_count MATCH (reused_1)-[]->(reused_2) RETURN edge_count, reused_1.val, reused_2.val ORDER BY reused_1.val, reused_2.val LIMIT 3"""
        executionPlan = graph.execution_plan(query)
        self.env.assertNotIn("Expand Into", executionPlan)
        resultset = graph.query(query).result_set
        expected = [[14, 0, 1],
                    [14, 0, 2],
                    [14, 0, 3]]
        self.env.assertEqual(resultset, expected)

    # Test limit propagation, execution-plan operations such as
    # conditional traverse accumulate a batch of records before processing
    # knowladge about limit can benifit such operation as they can reduce
    # their batch size to match the current limit.
    def test23_limit_propagation(self):
        graph_id = "limit-propagation"
        graph = Graph(redis_con, graph_id)

        # create graph
        query = """UNWIND range(0, 64) AS x CREATE ()-[:R]->()-[:R]->()"""
        graph.query(query)

        # query with LIMIT 1
        query = """CYPHER l=1 MATCH (a)-[]->(b) WITH b AS b
        MATCH (b)-[]->(c) RETURN c LIMIT $l"""

        # profile query
        profile = redis_con.execute_command("GRAPH.PROFILE", graph_id, query)
        profile = [x[0:x.index(',')].strip() for x in profile]

        # make sure 'a' to 'b' traversal operation is aware of limit
        self.env.assertIn("Conditional Traverse | (a)->(b) | Records produced: 1", profile)

        # query with LIMIT 1
        query = """CYPHER l=1 MATCH (a), (b) WITH a AS a, b AS b
        MATCH (a)-[]->(b) WITH b AS b MATCH (b)-[]->(c) RETURN c LIMIT $l"""

        # profile query
        profile = redis_con.execute_command("GRAPH.PROFILE", graph_id, query)
        profile = [x[0:x.index(',')].strip() for x in profile]

        # make sure 'a' to 'b' expand into traversal operation is aware of limit
        self.env.assertIn("Expand Into | (a)->(b) | Records produced: 1", profile)

        # aggregation should reset limit, otherwise we'll take a performance hit
        # recall aggregation operations are eager
        query = """CYPHER l=1 MATCH (a)-[]->(b) WITH count(a) AS src, b AS b
        MATCH (b)-[]->(c) RETURN c LIMIT $l"""

        # profile query
        profile = redis_con.execute_command("GRAPH.PROFILE", graph_id, query)
        profile = [x[0:x.index(',')].strip() for x in profile]

        # traversal from a to b shouldn't be effected by the limit.
        self.env.assertNotIn("Conditional Traverse | (a)->(b) | Records produced: 64", profile)

    # "WHERE true" predicates should not build filter ops.
    def test24_compact_true_predicates(self):
        query = """MATCH (a) WHERE true RETURN a"""
        executionPlan = graph.execution_plan(query)
        self.env.assertNotIn("Filter", executionPlan)

    # Cartesian product filter placement should not recurse into earlier scopes.
    def test25_optimize_cartesian_product_scoping(self):
        query = """MATCH (a {name: 'Ailon'})-[]->(b {name: 'Roi'}) WITH 'const' AS c MATCH (a), (b) WHERE a.val = 3 OR b.val = 3 RETURN a.val, b.val ORDER BY a.val, b.val LIMIT 3"""
        resultset = graph.query(query).result_set
        expected = [[0, 3],
                    [0, 3],
                    [0, 3]]
        self.env.assertEqual(resultset, expected)

    # Constant filters should not break Cartesian Product placement.
    def test26_optimize_cartesian_product_constant_filters(self):
        query = """MATCH (a) WHERE 2 > rand() MATCH (a), (b) RETURN a.val, b.val ORDER BY a.val, b.val DESC LIMIT 3"""
        resultset = graph.query(query).result_set
        expected = [[0, 3],
                    [0, 3],
                    [0, 3]]
        self.env.assertEqual(resultset, expected)

    # Filters on single Cartesian Product branches should be placed properly.
    def test27_optimize_cartesian_product_complex_filter_trees(self):
        query = """MATCH (a), (b), (c) WHERE a.val = 0 OR 'lit' > 3 AND b.val <> b.fake RETURN a.val, b.val ORDER BY a.val, b.val DESC LIMIT 3"""
        resultset = graph.query(query).result_set
        expected = [[0, 3],
                    [0, 3],
                    [0, 3]]
        self.env.assertEqual(resultset, expected)

    # Labels' order should be replaced properly.
    def test28_optimize_label_scan_switch_labels(self):
        self.env.flush()

        # Create three nodes with label N, two with label M, one of them in common.
        graph.query("CREATE (:N), (:N), (:N:M), (:M)")

        # Make sure that the M is traversed first.
        query = "MATCH (n:N:M) RETURN n"
        plan = graph.execution_plan(query)
        self.env.assertIn("Node By Label Scan | (n:M)", plan)

        # Make sure multi-label is enforced, we're expecting only the node with
        # both :N and :M to be returned.
        res = graph.query(query)
        self.env.assertEquals(len(res.result_set), 1)
        self.env.assertEquals(res.result_set[0][0], Node(alias='n', label=['N', 'M']))

    # in cases where a referred label doesn't exist, the UNKNOW_LABEL_ID 
    # is being cached. Once the label is created we want to make sure that 
    # the UNKNOW_LABEL_ID is replaced with the actual label ID. this test 
    # illustrates this scenario by traversing from a non-existing label 
    # (populating our execution-plan cache) which afterwards is being 
    # created. once created we want to make sure the correct label ID is used.
    def test29_optimize_label_scan_cached_label_id(self):
        self.env.flush()

        # Create node with label Q
        graph.query("CREATE (n:Q)")

        # Make sure N is traversed first, as it has no nodes. (none existing)
        plan = graph.execution_plan("MATCH (n:N:Q) RETURN n")
        self.env.assertIn("Node By Label Scan | (n:N)", plan)

        # Add label `N` to only node in the graph
        query = """MATCH (n:Q) SET n:N"""
        graph.query(query)

        # Make sure #nodes labeled as Q > #nodes labeled as N
        graph.query("CREATE (n:Q)")

        # Make sure N is traversed first, N is now associated with an ID
        # |N| < |Q|
        query = """MATCH (n:N:Q) RETURN count(n)"""
        res = graph.query(query)
        self.env.assertEquals(res.result_set, [[1]])

        plan = graph.execution_plan(query)
        self.env.assertIn("Node By Label Scan | (n:N)", plan)