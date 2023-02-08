from common import *
from index_utils import *

GRAPH_ID = "undo-log"

class testUndoLog():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        self.redis_con = self.env.getConnection()
        self.graph = Graph(self.redis_con, GRAPH_ID)

    def tearDown(self):
        self.redis_con.flushall()

    def test01_undo_create_node(self):
        try:
            self.graph.query("CREATE (n:N) WITH n RETURN 1 * n")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # node (n:N) should be removed, expecting an empty graph
        result = self.graph.query("MATCH (n:N) RETURN n")
        self.env.assertEquals(len(result.result_set), 0)

        # no label should be created
        result = self.graph.query("CALL db.labels")
        self.env.assertEquals(len(result.result_set), 0)


    def test02_undo_create_edge(self):
        self.graph.query("CREATE (:N {v: 1}), (:N {v: 2})")
        try:
            self.graph.query("""MATCH (s:N {v: 1}), (t:N {v: 2})
                                CREATE (s)-[r:R]->(t)
                                WITH r
                                RETURN 1 * r""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # edge [r:R] should have been removed
        result = self.graph.query("MATCH ()-[r:R]->() RETURN r")
        self.env.assertEquals(len(result.result_set), 0)

        # no relation should be added
        result = self.graph.query("CALL db.relationshipTypes")
        self.env.assertEquals(len(result.result_set), 0)

    def test03_undo_delete_node(self):
        self.graph.query("CREATE (:N)")
        try:
            self.graph.query("""MATCH (n:N)
                                DELETE n
                                WITH n
                                RETURN 1 * n""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # deleted node should be revived, expecting a single node
        result = self.graph.query("MATCH (n:N) RETURN n")
        self.env.assertEquals(len(result.result_set), 1)

    def test04_undo_delete_edge(self):
        self.graph.query("CREATE (:N)-[:R]->(:N)")
        try:
            self.graph.query("""MATCH ()-[r:R]->()
                                DELETE r
                                WITH r 
                                RETURN 1 * r""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # deleted edge should be revived, expecting a single edge
        result = self.graph.query("MATCH ()-[r:R]->() RETURN r")
        self.env.assertEquals(len(result.result_set), 1)

    def test05_undo_update_node(self):
        self.graph.query("CREATE (:N {a: 1, b:'str', c:[1, 'str', point({latitude:1, longitude:2})], d:point({latitude:1, longitude:2})})")
        property_keys = self.graph.query("CALL db.propertyKeys").result_set
        try:
            self.graph.query("""MATCH (n:N {a: 1})
                                SET n.a = 2, n.b = '', n.c = null, n.d = point({latitude:2, longitude:1})
                                WITH n
                                RETURN 1 * n""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # expecting the original attributes to be restored
        result = self.graph.query("MATCH (n:N) RETURN n.a, n.b, n.c, n.d")
        self.env.assertEquals(result.result_set[0][0], 1)
        self.env.assertEquals(result.result_set[0][1], 'str')
        self.env.assertEquals(result.result_set[0][2], [1, 'str', {'latitude':1, 'longitude':2}])
        self.env.assertEquals(result.result_set[0][3], {'latitude':1, 'longitude':2})

        # no new properties should have been created
        new_property_keys = self.graph.query("CALL db.propertyKeys").result_set
        self.env.assertEquals(property_keys, new_property_keys)

        # introduce a new attribute `n.e`
        try:
            self.graph.query("""MATCH (n:N {a: 1})
                                SET n.e = 1
                                WITH n
                                RETURN 1 * n""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # expecting the original attributes to be deleted
        result = self.graph.query("MATCH (n:N) RETURN n.e")
        self.env.assertEquals(result.result_set[0][0], None)

        # no new properties should have been created
        new_property_keys = self.graph.query("CALL db.propertyKeys").result_set
        self.env.assertEquals(property_keys, new_property_keys)

        # introduce a new Label `n:M`
        try:
            self.graph.query("""MATCH (n:N {a: 1})
                                SET n:M
                                WITH n
                                RETURN 1 * n""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # expecting the additional label 'M' to be removed
        result = self.graph.query("MATCH (n:M) RETURN COUNT(n)")
        self.env.assertEquals(result.result_set[0][0], 0)

        # clear all attributes of `n`
        try:
            self.graph.query("""MATCH (n:N {a: 1})
                                SET n = {}
                                WITH n
                                RETURN n * 1""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # expecting the original attributes to be restored
        result = self.graph.query("MATCH (n:N) RETURN n.a, n.b, n.c, n.d")
        self.env.assertEquals(result.result_set[0][0], 1)
        self.env.assertEquals(result.result_set[0][1], 'str')
        self.env.assertEquals(result.result_set[0][2], [1, 'str', {'latitude':1, 'longitude':2}])
        self.env.assertEquals(result.result_set[0][3], {'latitude':1, 'longitude':2})
        new_property_keys = self.graph.query("CALL db.propertyKeys").result_set
        self.env.assertEquals(property_keys, new_property_keys)

        try:
            self.graph.query("""MATCH (n:N {a: 1})
                                SET n += {e: 1}
                                WITH n
                                RETURN n * 1""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # expecting the original attributes to be restored
        result = self.graph.query("MATCH (n:N) RETURN n.a, n.b, n.c, n.d, n.e")
        self.env.assertEquals(result.result_set[0][0], 1)
        self.env.assertEquals(result.result_set[0][1], 'str')
        self.env.assertEquals(result.result_set[0][2], [1, 'str', {'latitude':1, 'longitude':2}])
        self.env.assertEquals(result.result_set[0][3], {'latitude':1, 'longitude':2})
        self.env.assertEquals(result.result_set[0][4], None)
        new_property_keys = self.graph.query("CALL db.propertyKeys").result_set
        self.env.assertEquals(property_keys, new_property_keys)

    def test06_undo_update_edge(self):
        self.graph.query("CREATE (:N)-[:R {v: 1}]->(:N)")
        property_keys = self.graph.query("CALL db.propertyKeys").result_set
        try:
            self.graph.query("""MATCH ()-[r]->()
                              SET r.v = 2
                              WITH r
                              RETURN r * 1""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # expecting the original attributes to be restored
        result = self.graph.query("MATCH ()-[r]->() RETURN r.v")
        self.env.assertEquals(result.result_set[0][0], 1)

        new_property_keys = self.graph.query("CALL db.propertyKeys").result_set
        self.env.assertEquals(property_keys, new_property_keys)

        try:
            self.graph.query("""MATCH ()-[r]->()
                              SET r.v2 = 2
                              WITH r
                              RETURN r * 1""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # expecting no values
        result = self.graph.query("MATCH ()-[r]->() RETURN r.v2")
        self.env.assertEquals(result.result_set, [[None]])

        new_property_keys = self.graph.query("CALL db.propertyKeys").result_set
        self.env.assertEquals(property_keys, new_property_keys)

    def test07_undo_create_indexed_node(self):
        create_node_exact_match_index(self.graph, "N", "v", sync=True)
        property_keys = self.graph.query("CALL db.propertyKeys").result_set
        try:
            self.graph.query("CREATE (n:N {v:1}) WITH n RETURN 1 * n")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # node (n:N) should be removed, expecting an empty graph
        result = self.graph.query("MATCH (n:N {v:1}) RETURN n")
        self.env.assertEquals(len(result.result_set), 0)
        # no new properties should have been created
        new_property_keys = self.graph.query("CALL db.propertyKeys").result_set
        self.env.assertEquals(property_keys, new_property_keys)

        try:
            self.graph.query("MERGE (n:N {v:1}) WITH n RETURN 1 * n")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # node (n:N) should be removed, expecting an empty graph
        result = self.graph.query("MATCH (n:N {v:1}) RETURN n")
        self.env.assertEquals(len(result.result_set), 0)
        # no new properties should have been created
        new_property_keys = self.graph.query("CALL db.propertyKeys").result_set
        self.env.assertEquals(property_keys, new_property_keys)

    def test08_undo_create_indexed_edge(self):
        create_edge_exact_match_index(self.graph, "R", "v", sync=True)
        self.graph.query("CREATE (:N {v: 1}), (:N {v: 2})")
        property_keys = self.graph.query("CALL db.propertyKeys").result_set
        try:
            self.graph.query("""MATCH (s:N {v: 1}), (t:N {v: 2})
                                CREATE (s)-[r:R {v:1}]->(t)
                                WITH r
                                RETURN 1 * r""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # edge [r:R] should have been removed
        result = self.graph.query("MATCH ()-[r:R {v:1}]->() RETURN r")
        self.env.assertEquals(len(result.result_set), 0)
        # no new properties should have been created
        new_property_keys = self.graph.query("CALL db.propertyKeys").result_set
        self.env.assertEquals(property_keys, new_property_keys)

        try:
            self.graph.query("""MATCH (s:N {v: 1}), (t:N {v: 2})
                                MERGE (s)-[r:R {v:1}]->(t)
                                WITH r
                                RETURN 1 * r""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # edge [r:R] should have been removed
        result = self.graph.query("MATCH ()-[r:R {v:1}]->() RETURN r")
        self.env.assertEquals(len(result.result_set), 0)
        # no new properties should have been created
        new_property_keys = self.graph.query("CALL db.propertyKeys").result_set
        self.env.assertEquals(property_keys, new_property_keys)

    def test09_undo_delete_indexed_node(self):
        create_node_exact_match_index(self.graph, "N", "v", sync=True)
        self.graph.query("CREATE (:N {v: 0})")
        try:
            self.graph.query("""MATCH (n:N)
                                DELETE n
                                WITH n
                                RETURN n * 1""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # deleted node should be revived, expecting a single node
        query = "MATCH (n:N {v: 0}) RETURN n"
        plan = self.graph.execution_plan(query)
        self.env.assertContains("Node By Index Scan", plan)
        result = self.graph.query(query)
        self.env.assertEquals(len(result.result_set), 1)

    def test10_undo_delete_indexed_edge(self):
        create_edge_exact_match_index(self.graph, "R", "v", sync=True)
        self.graph.query("CREATE (:N)-[:R {v: 0}]->(:N)")
        try:
            self.graph.query("""MATCH ()-[r:R]->()
                                DELETE r
                                WITH r
                                RETURN r * 1""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # deleted edge should be revived, expecting a single edge
        query = "MATCH ()-[r:R {v: 0}]->() RETURN r"
        plan = self.graph.execution_plan(query)
        self.env.assertContains("Edge By Index Scan", plan)
        result = self.graph.query(query)
        self.env.assertEquals(len(result.result_set), 1)

    def test11_undo_update_indexed_node(self):
        create_node_exact_match_index(self.graph, "N", "v", sync=True)
        self.graph.query("CREATE (:N {v: 1})")
        try:
            self.graph.query("""MATCH (n:N {v: 1})
                                SET n.v = 2
                                WITH n
                                RETURN n * 1""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # expecting the original attributes to be restored and indexed
        query = "MATCH (n:N {v: 1}) RETURN n.v"
        plan = self.graph.execution_plan(query)
        self.env.assertContains("Node By Index Scan", plan)
        result = self.graph.query(query)
        self.env.assertEquals(result.result_set[0][0], 1)
    
    def test12_undo_update_indexed_edge(self):
        create_edge_exact_match_index(self.graph, "R", "v", sync=True)
        self.graph.query("CREATE (:N)-[:R {v: 1}]->(:N)")
        try:
            self.graph.query("""MATCH ()-[r]->()
                                SET r.v = 2
                                WITH r
                                RETURN r * 1""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # expecting the original attributes to be restored and indexed
        query = "MATCH ()-[r:R {v: 1}]->() RETURN r.v"
        plan = self.graph.execution_plan(query)
        self.env.assertContains("Edge By Index Scan", plan)
        result = self.graph.query(query)
        self.env.assertEquals(result.result_set[0][0], 1)

    def test13_undo_implicit_edge_delete(self):
        self.graph.query("CREATE (n:N), (m:N), (n)-[:R]->(m), (n)-[:R]->(m)")
        try:
            self.graph.query("""MATCH (n:N)
                                DETACH DELETE n
                                WITH n
                                RETURN 1 * n""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # deleted node should be revived, expecting a single node
        result = self.graph.query("MATCH (n:N) RETURN n")
        self.env.assertEquals(len(result.result_set), 2)
        result = self.graph.query("MATCH ()-[r:R]->() RETURN r")
        self.env.assertEquals(len(result.result_set), 2)

    def test14_undo_timeout(self):
        # Change timeout value from default
        response = self.redis_con.execute_command("GRAPH.CONFIG SET TIMEOUT_DEFAULT 1")
        self.env.assertEqual(response, "OK")

        try:
            self.graph.query("UNWIND range(1, 1000000) AS x CREATE (n:N)")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except Exception as e:
            pass

        # Restore timeout value to default
        response = self.redis_con.execute_command("GRAPH.CONFIG SET TIMEOUT_DEFAULT 0")
        self.env.assertEqual(response, "OK")

        # node (n:N) should be removed, expecting an empty graph
        result = self.graph.query("MATCH (n:N) RETURN n")
        self.env.assertEquals(len(result.result_set), 0)


    def test15_complex_undo(self):
        # create a graph
        self.graph.query("UNWIND range(1, 3) AS x CREATE (:N {v:x})-[:R{v:x}]->(:N {v:x})")

        try:
            self.graph.query("MATCH (n:N)-[r:R]->(m:N) SET n.v = n.v + 1, r.v = r.v + 1, m.v = m.v + 1 CREATE (:N{v:n.v}) DELETE r RETURN CASE n.v WHEN 3 THEN n.v * 'a' ELSE n.v END")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except Exception as e:
            self.env.assertEquals(str(e), "Type mismatch: expected Integer, Float, or Null but was String")

        # validate no changed is the created graph
        expected_result = [[1, 1, 1], [2, 2, 2], [3, 3, 3]]
        result = self.graph.query("MATCH (n:N)-[r:R]->(m:N) RETURN n.v, r.v, m.v")
        self.env.assertEquals(result.result_set, expected_result)


    def test16_undo_label_set(self):
        create_node_exact_match_index(self.graph, "L1", "v", sync=True)
        self.graph.query("CREATE (n:L1 {v:1})")
        try:
            self.graph.query("MATCH (n:L1) SET n:L2 WITH n RETURN 1 * n")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # node label L2 should not be added, expecting an empty result set
        result = self.graph.query("MATCH (n:L2) RETURN n")
        self.env.assertEquals(len(result.result_set), 0)
        # check index is ok
        query = "MATCH (n:L1 {v: 1}) RETURN n.v"
        plan = self.graph.execution_plan(query)
        self.env.assertContains("Node By Index Scan", plan)
        result = self.graph.query(query)
        self.env.assertEquals(result.result_set[0][0], 1)

        # L2 label should not be created
        result = self.graph.query("CALL db.labels")
        self.env.assertEquals(result.result_set, [["L1"]])

    def test17_undo_remove_label(self):
        create_node_exact_match_index(self.graph, "L2", "v", sync=True)
        self.graph.query("CREATE (n:L2 {v:1})")
        try:
            self.graph.query("MATCH (n:L2) REMOVE n:L2 WITH n RETURN 1 * n")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # node label L2 not be removed
        result = self.graph.query("MATCH (n:L2) RETURN labels(n)")
        self.env.assertEquals(len(result.result_set), 1)
        self.env.assertEquals(["L2"], result.result_set[0][0])
        # check index is ok
        query = "MATCH (n:L2 {v: 1}) RETURN n.v"
        plan = self.graph.execution_plan(query)
        self.env.assertContains("Node By Index Scan", plan)
        result = self.graph.query(query)
        self.env.assertEquals(result.result_set[0][0], 1)

    def test18_undo_set_remove_label(self):
        self.graph.query("CREATE (n:L3)")
        try:
            self.graph.query("MATCH (n:L3) SET n:L4 REMOVE n:L3 WITH n RETURN 1 * n")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # node label L3 should not be removed, L4 should bot be created
        result = self.graph.query("MATCH (n:L3) RETURN labels(n)")
        self.env.assertEquals(len(result.result_set), 1)
        self.env.assertEquals(["L3"], result.result_set[0][0])

    def test19_undo_remove_set_label(self):
        self.graph.query("CREATE (n:L4)")
        try:
            self.graph.query("MATCH (n:L4) REMOVE n:L4 SET n:L5 WITH n RETURN 1 * n")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # node label L4 should not be removed, L5 should bot be created
        result = self.graph.query("MATCH (n:L4) RETURN labels(n)")
        self.env.assertEquals(len(result.result_set), 1)
        self.env.assertEquals(["L4"], result.result_set[0][0])
